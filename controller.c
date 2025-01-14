/**
 * controller.c
 *
 * Versão corrigida para evitar erros de compilação relacionados à definição incorreta
 * da função 'main' e parâmetros não utilizados.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>

// >>> Adicionados para GPIO e PWM <<<
#include <wiringPi.h>
#include <softPwm.h>

// Definições de chaves IPC
#define SHM_KEY_SENSORS 1234
#define SHM_KEY_TRIGGERS 4321
#define MSG_KEY 5678

// >>> Ajustar conforme o seu diagrama de hardware <<<
#define MOTOR_DIR1 17  // GPIO 17 (BCM)
#define MOTOR_DIR2 18  // GPIO 18 (BCM)
#define MOTOR_POT 23   // GPIO 23 (BCM) -> PWM para motor
#define FREIO_INT 24   // GPIO 24 (BCM) -> PWM para freio

// Exemplo de pinos de faróis e setas
#define FAROL_BAIXO 19
#define FAROL_ALTO  26
#define LUZ_SETA_ESQ 8
#define LUZ_SETA_DIR 7

// Sensores Hall
#define SENSOR_HALL_MOTOR 11
#define SENSOR_HALL_RODA_A 5
#define SENSOR_HALL_RODA_B 6

// Definições de constantes para cálculo de temperatura
#define FACTOR_ACELERACAO 0.1 
#define FATOR_RESFRIAMENTO_AR 0.05
#define MAX_TEMP_MOTOR 140
#define BASE_TEMP 80

// Estrutura para os dados dos sensores
typedef struct {
    float velocidade; 
    int rpm;          
    float temperatura;
} SensorData;

// Estrutura para o status dos acionadores
typedef struct {
    bool seta_dir, seta_esq;
    bool farol_baixo, farol_alto;
} Status_trigg;

// Estrutura para mensagens do painel
typedef struct {
    long msg_type;
    char command[100];
} Message;

// Variáveis globais
SensorData *shared_data;      
Status_trigg *status_trigg;
int shm_id_sensors, shm_id_triggers;
int msg_queue_id;
sem_t *sem_sync;
volatile sig_atomic_t running = 1; 

// >>> Variáveis para PWM e Contadores <<<
static int motorDuty = 0;   // Duty cycle motor (0-100)
static int freioDuty = 0;   // Duty cycle freio (0-100)

// Contadores para sensor Hall (RPM, velocidade)
static volatile unsigned long motorPulsos = 0;  
static volatile unsigned long rodaPulsos = 0;   

// >>> Relatórios (do seu código original) <<<
int cont_vel_sup = 0;
int cont_vel_inf = 0;
int cont_rpm_sup = 0;
int cont_rpm_inf = 0;
int cont_max_temp = 0;

// --------------------------
// 1. Funções de Callback para Sensores Hall
// --------------------------
void motor_hall_callback(void) {
    motorPulsos++;
}

void roda_a_hall_callback(void) {
    rodaPulsos++;
}

// --------------------------
// 2. Tratamento de Sinais
// --------------------------
void signal_handler(int signal) {
    if (signal == SIGUSR1) {
        printf("Teste pausado (SIGUSR1)\n");
        sem_wait(sem_sync); 
    } else if (signal == SIGUSR2) {
        printf("Encerrando o programa (SIGUSR2)\n");
        
        // Enviar mensagem "Encerrar" ao Painel de Comando
        Message msg;
        msg.msg_type = 2; 
        strcpy(msg.command, "Encerrar");
        if (msgsnd(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
            perror("Erro ao enviar mensagem de encerramento para o Painel");
        }
        running = 0; // Sinaliza para encerrar
    }
}

// >>> Novo: tratar SIGINT para desligar PWM e GPIO <<<
void sigint_handler_custom(int sig) {
    if (sig == SIGINT) {
        printf("\nRecebido Ctrl + C (SIGINT). Encerrando...\n");
        
        // Enviar "Encerrar" ao painel também
        Message msg;
        msg.msg_type = 2;
        strcpy(msg.command, "Encerrar");
        if (msgsnd(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
            perror("Erro ao enviar mensagem de encerramento para o Painel");
        }

        running = 0;
    }
}

void setup_signals() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Erro ao instalar handler SIGUSR1");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Erro ao instalar handler SIGUSR2");
        exit(EXIT_FAILURE);
    }

    // Adicionar SIGINT com outro handler
    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler_custom;
    sa_int.sa_flags = SA_RESTART;
    sigemptyset(&sa_int.sa_mask);
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("Erro ao instalar handler SIGINT");
        exit(EXIT_FAILURE);
    }
}

// --------------------------
// 3. Memória Compartilhada
// --------------------------
void init_shared_memory() {
    // SensorData
    shm_id_sensors = shmget(SHM_KEY_SENSORS, sizeof(SensorData), IPC_CREAT | 0666);
    if (shm_id_sensors < 0) {
        perror("Erro ao criar memória para sensores");
        exit(EXIT_FAILURE);
    }
    shared_data = (SensorData *)shmat(shm_id_sensors, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Erro ao associar memória para sensores");
        shmctl(shm_id_sensors, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Status_trigg
    shm_id_triggers = shmget(SHM_KEY_TRIGGERS, sizeof(Status_trigg), IPC_CREAT | 0666);
    if (shm_id_triggers < 0) {
        perror("Erro ao criar memória para acionadores");
        shmctl(shm_id_sensors, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    status_trigg = (Status_trigg *)shmat(shm_id_triggers, NULL, 0);
    if (status_trigg == (void *)-1) {
        perror("Erro ao associar memória para acionadores");
        shmctl(shm_id_sensors, IPC_RMID, NULL);
        shmctl(shm_id_triggers, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Inicializar valores
    shared_data->velocidade = 0.0;
    shared_data->rpm = 800;
    shared_data->temperatura = 0.0;

    status_trigg->seta_dir = false;
    status_trigg->seta_esq = false;
    status_trigg->farol_baixo = false;
    status_trigg->farol_alto = false;

    printf("Memórias compartilhadas inicializadas.\n");
}

// --------------------------
// 4. Fila de Mensagens
// --------------------------
void init_message_queue() {
    msg_queue_id = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msg_queue_id < 0) {
        perror("Erro ao criar fila de mensagens");
        exit(EXIT_FAILURE);
    }
    
    // Limpar mensagens residuais
    Message msg;
    while (msgrcv(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0, IPC_NOWAIT) > 0) {
        // descartar
    }
}

// --------------------------
// 5. Semáforo
// --------------------------
void init_semaphore() {
    sem_unlink("/sem_sync");
    sem_sync = sem_open("/sem_sync", O_CREAT | O_EXCL, 0666, 1);
    if (sem_sync == SEM_FAILED) {
        perror("Erro ao criar semáforo");
        exit(EXIT_FAILURE);
    }
}

// --------------------------
// 6. Cálculo de Temperatura
// --------------------------
float calculate_engine_temp(float velocidade, int rpm) {
    float temp_rise = (rpm / 10.0) * FACTOR_ACELERACAO;
    float cooling_effect = velocidade * FATOR_RESFRIAMENTO_AR;
    float temp = BASE_TEMP + temp_rise - cooling_effect;
    return (float)fmin(MAX_TEMP_MOTOR, temp);
}

// --------------------------
// 7. GPIO e PWM
// --------------------------
void motor_set_direction(char direction) {
    // 'D' = frente, 'R' = ré, 'B' = freio, 'N' = neutro
    switch (direction) {
        case 'D':
            digitalWrite(MOTOR_DIR1, HIGH);
            digitalWrite(MOTOR_DIR2, LOW);
            break;
        case 'R':
            digitalWrite(MOTOR_DIR1, LOW);
            digitalWrite(MOTOR_DIR2, HIGH);
            break;
        case 'B':
            digitalWrite(MOTOR_DIR1, HIGH);
            digitalWrite(MOTOR_DIR2, HIGH);
            break;
        case 'N':
        default:
            digitalWrite(MOTOR_DIR1, LOW);
            digitalWrite(MOTOR_DIR2, LOW);
            break;
    }
}

void init_gpio() {
    // Inicializar WiringPi (modo BCM)
    if (wiringPiSetupGpio() < 0) {
        fprintf(stderr, "Erro ao inicializar WiringPi\n");
        exit(EXIT_FAILURE);
    }

    // Configurar pinos de direção do motor
    pinMode(MOTOR_DIR1, OUTPUT);
    pinMode(MOTOR_DIR2, OUTPUT);

    // Configurar PWM do motor e do freio
    // softPwmCreate(pino, valor_inicial, range)
    // 'range' ~ 100 => duty cycle 0-100
    if (softPwmCreate(MOTOR_POT, 0, 100) != 0) {
        fprintf(stderr, "Erro ao criar PWM para MOTOR_POT\n");
        exit(EXIT_FAILURE);
    }
    if (softPwmCreate(FREIO_INT, 0, 100) != 0) {
        fprintf(stderr, "Erro ao criar PWM para FREIO_INT\n");
        exit(EXIT_FAILURE);
    }

    // Faróis / Seta (saídas digitais)
    pinMode(FAROL_BAIXO, OUTPUT);
    pinMode(FAROL_ALTO, OUTPUT);
    pinMode(LUZ_SETA_ESQ, OUTPUT);
    pinMode(LUZ_SETA_DIR, OUTPUT);

    // Sensores Hall (entradas)
    pinMode(SENSOR_HALL_MOTOR, INPUT);
    pinMode(SENSOR_HALL_RODA_A, INPUT);
    pinMode(SENSOR_HALL_RODA_B, INPUT);

    // Puxar para baixo (pull-down) se necessário
    pullUpDnControl(SENSOR_HALL_MOTOR, PUD_DOWN);
    pullUpDnControl(SENSOR_HALL_RODA_A, PUD_DOWN);
    pullUpDnControl(SENSOR_HALL_RODA_B, PUD_DOWN);

    // Configurar interrupções para sensor Hall
    wiringPiISR(SENSOR_HALL_MOTOR, INT_EDGE_RISING, &motor_hall_callback);
    wiringPiISR(SENSOR_HALL_RODA_A, INT_EDGE_RISING, &roda_a_hall_callback);
    // SENSOR_HALL_RODA_B poderia ser usado para detectar direção

    printf("GPIO inicializados.\n");
}

// --------------------------
// 8. Thread para Piscar Setas
// --------------------------
void *threadPiscaSetaEsq(void *arg) {
    (void)arg; // Silenciar warning de parâmetro não utilizado

    while (running) {
        sem_wait(sem_sync);
        bool ligada = status_trigg->seta_esq;
        sem_post(sem_sync);

        if (ligada) {
            digitalWrite(LUZ_SETA_ESQ, HIGH);
            sleep(1);
            digitalWrite(LUZ_SETA_ESQ, LOW);
            sleep(1);
        } else {
            // Se seta não estiver ativa, garante desligado
            digitalWrite(LUZ_SETA_ESQ, LOW);
            usleep(200000); 
        }
    }
    return NULL;
}

void *threadPiscaSetaDir(void *arg) {
    (void)arg; // Silenciar warning de parâmetro não utilizado

    while (running) {
        sem_wait(sem_sync);
        bool ligada = status_trigg->seta_dir;
        sem_post(sem_sync);

        if (ligada) {
            digitalWrite(LUZ_SETA_DIR, HIGH);
            sleep(1);
            digitalWrite(LUZ_SETA_DIR, LOW);
            sleep(1);
        } else {
            digitalWrite(LUZ_SETA_DIR, LOW);
            usleep(200000);
        }
    }
    return NULL;
}

// --------------------------
// 9. Loop Principal
// --------------------------
void process_control() {
    // Criar threads para piscar setas
    pthread_t th_esq, th_dir;
    if (pthread_create(&th_esq, NULL, threadPiscaSetaEsq, NULL) != 0) {
        perror("Erro ao criar thread para piscar seta esquerda");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&th_dir, NULL, threadPiscaSetaDir, NULL) != 0) {
        perror("Erro ao criar thread para piscar seta direita");
        exit(EXIT_FAILURE);
    }

    // Loop principal
    while (running) {
        float aux_vel, aux_temp;
        int aux_rpm;

        sem_wait(sem_sync);
        aux_vel = shared_data->velocidade;
        aux_rpm = shared_data->rpm;
        aux_temp = shared_data->temperatura;
        sem_post(sem_sync);

        // Mostrar dados
        printf("\n===== Dados dos Sensores =====\n");
        printf("Velocidade: %.1f km/h\n", aux_vel);
        printf("RPM: %d\n", aux_rpm);
        printf("Temperatura: %.2f ºC\n", aux_temp);

        // Regras de limite
        if (aux_vel > 200.0) {
            aux_vel *= 0.9; 
            cont_vel_sup++;
        } else if (aux_vel < 20.0 && aux_vel > 0.0) {
            aux_vel *= 1.1; 
            cont_vel_inf++;
        }

        if (aux_rpm > 8000) {
            aux_rpm *= 0.9;
            cont_rpm_sup++;
        } else if (aux_rpm < 800) {
            aux_rpm = 0; 
            cont_rpm_inf++;
            printf("\n========= O motor apagou =========\n");
            raise(SIGUSR2);
        } else if (aux_temp >= 140.0) {
            printf("\n========= ALERTA DE TEMPERATURA =========\n");
            cont_max_temp++;
            aux_vel *= 0.9;
            aux_rpm *= 0.9;
        }

        // Atualizar memória
        sem_wait(sem_sync);
        shared_data->velocidade  = aux_vel;
        shared_data->rpm         = aux_rpm;
        shared_data->temperatura = calculate_engine_temp(aux_vel, aux_rpm);
        sem_post(sem_sync);

        // Exibir status das luzes
        sem_wait(sem_sync);
        printf("\n===== Dados dos Acionadores =====\n");
        printf("Seta Direita: %s\n", status_trigg->seta_dir ? "Ligado" : "Desligado");
        printf("Seta Esquerda: %s\n", status_trigg->seta_esq ? "Ligado" : "Desligado");
        printf("Farol Baixo: %s\n", status_trigg->farol_baixo ? "Ligado" : "Desligado");
        printf("Farol Alto: %s\n", status_trigg->farol_alto ? "Ligado" : "Desligado");
        sem_post(sem_sync);

        // Ler comandos do painel
        Message msg;
        if (msgrcv(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 1, IPC_NOWAIT) > 0) {
            printf("\nComando recebido do Painel: %s\n", msg.command);

            // Processar comando
            if (strcmp(msg.command, "Ligar Seta Esquerda") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_esq = true;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Desligar Seta Esquerda") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_esq = false;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Ligar Seta Direita") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_dir = true;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Desligar Seta Direita") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_dir = false;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Ligar Farol Baixo") == 0) {
                digitalWrite(FAROL_BAIXO, HIGH);
                sem_wait(sem_sync);
                status_trigg->farol_baixo = true;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Desligar Farol Baixo") == 0) {
                digitalWrite(FAROL_BAIXO, LOW);
                sem_wait(sem_sync);
                status_trigg->farol_baixo = false;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Ligar Farol Alto") == 0) {
                digitalWrite(FAROL_ALTO, HIGH);
                sem_wait(sem_sync);
                status_trigg->farol_alto = true;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Desligar Farol Alto") == 0) {
                digitalWrite(FAROL_ALTO, LOW);
                sem_wait(sem_sync);
                status_trigg->farol_alto = false;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Desligar Farol") == 0) {
                digitalWrite(FAROL_BAIXO, LOW);
                digitalWrite(FAROL_ALTO, LOW);
                sem_wait(sem_sync);
                status_trigg->farol_baixo = false;
                status_trigg->farol_alto = false;
                sem_post(sem_sync);
            }
            else if (strcmp(msg.command, "Acionar Pedal do Acelerador") == 0) {
                // Aumentar duty cycle do motor
                if (motorDuty < 100) motorDuty += 10; 
                if (motorDuty > 100) motorDuty = 100;
                softPwmWrite(MOTOR_POT, motorDuty);

                // Ajustar direção para frente
                motor_set_direction('D');
            }
            else if (strcmp(msg.command, "Acionar Pedal do Freio") == 0) {
                // Aumentar duty cycle do freio
                if (freioDuty < 100) freioDuty += 10;
                if (freioDuty > 100) freioDuty = 100;
                softPwmWrite(FREIO_INT, freioDuty);

                // Opcional: setar motor em 'B' (freio ativo)
                // motor_set_direction('B');
            }
            else if (strcmp(msg.command, "Encerrar") == 0) {
                raise(SIGUSR2);
            }
        }

        // Calcular RPM e Velocidade reais a partir de motorPulsos / rodaPulsos
        // Implementar lógica baseada no tempo para converter pulsos em RPM e velocidade
        // Exemplo simplificado:
        // Resetar contadores a cada intervalo de tempo e calcular os valores

        // Simular tempo de processamento
        sleep(1);
    }
}

// --------------------------
// 10. Cleanup
// --------------------------
void cleanup() {
    static bool cleaned = false;
    if (cleaned) return; 
    cleaned = true;

    printf("Limpando recursos...\n");

    // Zerar PWM
    softPwmWrite(MOTOR_POT, 0);
    softPwmWrite(FREIO_INT, 0);
    motor_set_direction('N');

    // Desligar faróis e setas
    digitalWrite(FAROL_BAIXO, LOW);
    digitalWrite(FAROL_ALTO, LOW);
    digitalWrite(LUZ_SETA_ESQ, LOW);
    digitalWrite(LUZ_SETA_DIR, LOW);

    // Desanexar e remover memória compartilhada
    if (shared_data) {
        shmdt(shared_data);
        shmctl(shm_id_sensors, IPC_RMID, NULL);
    }
    if (status_trigg) {
        shmdt(status_trigg);
        shmctl(shm_id_triggers, IPC_RMID, NULL);
    }

    // Fechar semáforo
    if (sem_sync) {
        sem_close(sem_sync);
        sem_unlink("/sem_sync");
    }

    printf("Recursos liberados com sucesso!\n");
}

// --------------------------
// 11. Main
// --------------------------
int main() {
    setup_signals();

    // Inicializar IPC
    init_shared_memory();
    init_message_queue();
    init_semaphore();

    // Inicializar GPIO e PWM
    init_gpio();

    printf("Controlador inicializado. Aguardando dados...\n");

    // Executar loop principal
    process_control();

    // Exibir relatório
    printf("\n======== RELATÓRIO DOS LIMITADORES ===========\n\n");
    printf("Limite superior da velocidade %d vezes.\n", cont_vel_sup);
    printf("Limite inferior da velocidade %d vezes.\n", cont_vel_inf);
    printf("Limite superior do RPM %d vezes.\n", cont_rpm_sup);
    printf("Limite inferior do RPM %d vezes.\n", cont_rpm_inf);
    printf("Limite de temperatura %d vezes.\n", cont_max_temp);
    printf("Acionamentos Totais: %d.\n",
           (cont_vel_sup + cont_vel_inf + cont_rpm_sup + cont_rpm_inf + cont_max_temp));
    printf("===================================================\n\n");

    // Limpar recursos antes de sair
    cleanup();

    return 0;
}
