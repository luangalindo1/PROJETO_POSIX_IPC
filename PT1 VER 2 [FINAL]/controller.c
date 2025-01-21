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

#define SHM_KEY_SENSORS 1234      // Chave para os dados dos sensores
#define SHM_KEY_TRIGGERS 4321     // Chave para o status dos acionadores
#define MSG_KEY 5678              // Chave da fila de mensagens

// Definições de constantes da função de cálculo da temperatura do motor
#define FACTOR_ACELERACAO 0.1 
#define FATOR_RESFRIAMENTO_AR 0.05
#define MAX_TEMP_MOTOR 140
#define BASE_TEMP 80

// Estrutura para os dados
typedef struct {
    float velocidade; // Velocidade do carro (km/h)
    int rpm;        // Rotação do motor (RPM)
    float temperatura; // Temperatura do motor
} SensorData;

// Estrutura para o status dos acionadores
typedef struct {
    bool seta_dir, seta_esq, farol_baixo, farol_alto;
} Status_trigg;

// Estrutura para mensagens do painel
typedef struct {
    long msg_type; // Tipo da mensagem (1 para comandos)
    char command[100]; // Comando enviado pelo painel
} Message;

// Variáveis globais
SensorData *shared_data;      // Ponteiro para os dados dos sensores
Status_trigg *status_trigg;   // Ponteiro para o status dos acionadores
int shm_id_sensors, shm_id_triggers; // IDs das memórias compartilhadas
int msg_queue_id;             // ID da fila de mensagens
sem_t *sem_sync;              // Semáforo para sincronização
volatile sig_atomic_t running = 1; // Variável para controlar execução do programa

// Variáveis de relatório
int cont_vel_sup = 0;
int cont_vel_inf = 0;
int cont_rpm_sup = 0;
int cont_rpm_inf = 0;
int cont_max_temp = 0;


/**
 * @brief Função callback para tratar sinais recebidos pelo programa.
 * 
 * Trata os sinais SIGUSR1 e SIGUSR2.
 * 
 * - Se o sinal for SIGUSR1: pausa o loop principal do programa, aguardando no semáforo.
 * - Se o sinal for SIGUSR2: envia uma mensagem "Encerrar" para o Painel de Comando e 
 *   sinaliza para encerrar o programa.
 */
void signal_handler(int signal) {
    if (signal == SIGUSR1) {
        printf("Teste pausado (SIGUSR1 recebido)\n");
        sem_wait(sem_sync); // Pausa o controlador (espera no semáforo)
    } else if (signal == SIGUSR2) {
        printf("Encerrando o programa (SIGUSR2 recebido)\n");
        
        // Enviar mensagem de encerramento para o Painel de Comando
        Message msg;
        msg.msg_type = 2; // Tipo da mensagem do Controlador
        strcpy(msg.command, "Encerrar");
        if (msgsnd(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
            perror("Erro ao enviar mensagem de encerramento para o Painel de Comando");
        }
        running = 0; // Sinaliza o encerramento do programa
    }
}


/**
 * @brief Instala os handlers para os sinais SIGUSR1 e SIGUSR2.
 *
 * SIGUSR1: Pausa o loop principal do programa.
 * SIGUSR2: Encerra o programa e envia uma mensagem "Encerrar" para o Painel de Comando.
 */
void setup_signals() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
}


/**
 * @brief Cria e inicializa memórias compartilhadas para sensores e acionadores.
 *
 * Cria memória compartilhada para SensorData e Status_trigg, associa-as e
 * inicializa os campos com valores padrão.
 *
 * @return Nada.
 */
void init_shared_memory() {
    // Criar memória compartilhada para SensorData
    shm_id_sensors = shmget(SHM_KEY_SENSORS, sizeof(SensorData), IPC_CREAT | 0666);
    if (shm_id_sensors < 0) {
        perror("Erro ao criar memória compartilhada para sensores");
        exit(EXIT_FAILURE);
    }
    shared_data = (SensorData *)shmat(shm_id_sensors, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Erro ao associar memória compartilhada para sensores");
        shmctl(shm_id_sensors, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Criar memória compartilhada para Status_trigg
    shm_id_triggers = shmget(SHM_KEY_TRIGGERS, sizeof(Status_trigg), IPC_CREAT | 0666);
    if (shm_id_triggers < 0) {
        perror("Erro ao criar memória compartilhada para acionadores");
        shmctl(shm_id_sensors, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    status_trigg = (Status_trigg *)shmat(shm_id_triggers, NULL, 0);
    if (status_trigg == (void *)-1) {
        perror("Erro ao associar memória compartilhada para acionadores");
        shmctl(shm_id_sensors, IPC_RMID, NULL);
        shmctl(shm_id_triggers, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Inicializar valores nas memórias compartilhadas
    shared_data->velocidade = 0.0;
    shared_data->rpm = 800;
    shared_data->temperatura = 0.0;

    status_trigg->seta_dir = false;
    status_trigg->seta_esq = false;
    status_trigg->farol_baixo = false;
    status_trigg->farol_alto = false;

    printf("Memórias compartilhadas inicializadas com sucesso.\n");
}

/**
 * @brief Cria a fila de mensagens para comunicação com o painel e remove
 *        mensagens residuais.
 *
 * Cria a fila de mensagens com a chave MSG_KEY e remove mensagens residuais
 * que por ventura tenham sido enviadas anteriormente.
 *
 * @return Nada.
 */
void init_message_queue() {
    msg_queue_id = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msg_queue_id < 0) {
        perror("Erro ao criar fila de mensagens");
        exit(EXIT_FAILURE);
    }
    
    // Remover todas as mensagens residuais
    Message msg;
    while (msgrcv(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0, IPC_NOWAIT) > 0) {
        // Removendo mensagens silenciosamente
    }
}


/**
 * @brief Inicializa o semáforo de sincronização entre threads
 *
 * Limpa o nome do semáforo e o cria com o valor inicial de 1.
 *
 * @return Nenhum
 */
void init_semaphore() {
    sem_unlink("/sem_sync");
    sem_sync = sem_open("/sem_sync", O_CREAT | O_EXCL, 0666, 1);
    if (sem_sync == SEM_FAILED) {
        perror("Erro ao criar semáforo");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Calcula a temperatura do motor com base na fórmula dada no enunciado
 *        do trabalho.
 *
 * @param velocidade A velocidade atual do veículo em km/h
 * @param rpm O valor do RPM do motor
 * @return A temperatura do motor em graus Celsius
 */
float calculate_engine_temp(float velocidade, int rpm) {
    float temp_rise = rpm/10 * FACTOR_ACELERACAO;
    float cooling_effect = velocidade * FATOR_RESFRIAMENTO_AR;
    float temp = BASE_TEMP + temp_rise - cooling_effect;
    return (float)fmin(MAX_TEMP_MOTOR, temp);
}

/**
 * @brief Função principal de controle do veículo.
 *
 * A função process_control() é a principal responsável pelo controle do veículo.
 * Ela executa um loop principal que monitora dados de sensores como velocidade, RPM
 * e temperatura, aplicando regras de segurança e limites. Além disso, a função
 * processa comandos recebidos do painel de controle, permitindo a interação com
 * diversos acionadores, como setas, faróis, e pedais do veículo.
 *
 * A função garante a correta sincronização de dados compartilhados utilizando
 * semáforos e gerencia o estado dos acionadores do veículo.
 */
void process_control() {
    while (running) {
        float aux_vel, aux_temp;
        int aux_rpm;
        
        sem_wait(sem_sync); // Garantir exclusão mútua

        // Ler dados dos sensores da memória compartilhada
        aux_vel = shared_data->velocidade;
        aux_rpm = shared_data->rpm;
        aux_temp = shared_data->temperatura;

        sem_post(sem_sync);
        
        // Exibir dados dos sensores
        printf("\n===== Dados dos Sensores =====\n");
        printf("Velocidade: %.0f km/h\n", aux_vel);
        printf("RPM: %d\n", aux_rpm);
        printf("Temperatura: %.2f ºC\n", aux_temp);

        // Iniciar limitadores de valores proibidos
        if (aux_vel > 200.0){
            aux_vel *= 0.9; // Desacelerar 10%
            cont_vel_sup++;
        } else if (aux_vel < 20.0){
            aux_vel *= 1.1; // Acelerar 10%
            cont_vel_inf++;
        }
        if (aux_rpm > 8000){
            aux_rpm *= 0.9; // o motor deve "cortar"
            cont_rpm_sup++;
        } else if (aux_rpm < 800){
            aux_rpm = 0; 
            cont_rpm_inf++;
            printf("\n========= O motor apagou =========\n");
            raise(SIGUSR2);
        } else if (aux_temp >= 140.0){
            printf("\n========= ALERTA DE TEMPERATURA =========\n");
            cont_max_temp++;
            aux_vel *= 0.9;
            aux_rpm *= 0.9;
        }

        sem_wait(sem_sync); // Garantir exclusão mútua

        // Atualizar dados dos sensores na memória compartilhada
        shared_data->velocidade = aux_vel;
        shared_data->rpm = aux_rpm;
        shared_data->temperatura = calculate_engine_temp(aux_vel, aux_rpm);

        // Exibir dados dos acionadores
        printf("\n===== Dados dos Acionadores =====\n");
        printf("Seta Direita: %s\n", status_trigg->seta_dir ? "Ligado" : "Desligado");
        printf("Seta Esquerda: %s\n", status_trigg->seta_esq ? "Ligado" : "Desligado");
        printf("Farol Baixo: %s\n", status_trigg->farol_baixo ? "Ligado" : "Desligado");
        printf("Farol Alto: %s\n", status_trigg->farol_alto ? "Ligado" : "Desligado");
        
        sem_post(sem_sync);

        // Ler comandos do painel (fila de mensagens)
        Message msg;
        if (msgrcv(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 1, IPC_NOWAIT) > 0) {
            printf("\nComando recebido do Painel: %s\n", msg.command);

            // Processar o comando recebido
            if (strcmp(msg.command, "Ligar Seta Esquerda") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_esq = true;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Seta Esquerda") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_esq = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Ligar Seta Direita") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_dir = true;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Seta Direita") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_dir = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Ligar Farol Baixo") == 0) {
                sem_wait(sem_sync);
                status_trigg->farol_baixo = true;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Farol Baixo") == 0) {
                sem_wait(sem_sync);
                status_trigg->farol_baixo = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Ligar Farol Alto") == 0) {
                sem_wait(sem_sync);
                status_trigg->farol_alto = true;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Farol Alto") == 0) {
                sem_wait(sem_sync);
                status_trigg->farol_alto = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Farol") == 0) {
                sem_wait(sem_sync);
                status_trigg->farol_baixo = false;
                status_trigg->farol_alto = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Acionar Pedal do Acelerador") == 0) {
                sem_wait(sem_sync);
                if (shared_data->velocidade <= 200.0){
                    shared_data->velocidade += 10.0; // Aumentar a velocidade em 10 km/h
                    shared_data->rpm += 200; // Aumentar o RPM em 200
                    shared_data->temperatura = calculate_engine_temp(shared_data->velocidade, shared_data->rpm);
                }
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Acionar Pedal do Freio") == 0) {
                sem_wait(sem_sync);
                if (shared_data->velocidade > 10.0){
                    shared_data->velocidade -= 10.0; // Diminuir a velocidade em 10 km/h
                    shared_data->rpm -= 200; // Diminuir o RPM em 200
                    shared_data->temperatura = calculate_engine_temp(shared_data->velocidade, shared_data->rpm);
                } else {
                    shared_data->velocidade = 0.0;
                    shared_data->rpm = 800;
                    shared_data->temperatura = calculate_engine_temp(shared_data->velocidade, shared_data->rpm);
                }
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Encerrar") == 0){
                raise(SIGUSR2);
            }
        }

        // Simular tempo de processamento
        sleep(1);
    }
}


/*
 * @brief Libera todos os recursos alocados pelo programa.
 *
 * Esta função garante a limpeza segura dos recursos alocados durante
 * a execução do programa. Os recursos tratados incluem:
 *  - Memória compartilhada utilizada para armazenar dados dos sensores
 *    e acionadores (SensorData e Status_trigg).
 *  - Semáforo usado para sincronização entre threads.
 *
 * A função é protegida contra múltiplas execuções usando uma verificação
 * interna, garantindo que os recursos sejam liberados apenas uma vez.
 * 
 * Observação:
 *  - Não há tratamento explícito para falhas ao liberar recursos.
 *  - Caso algum recurso já tenha sido liberado externamente, a função
 *    pode não detectar isso, mas seguirá o fluxo sem interrupções.
 *
 * @return Nada.
 */
void cleanup() {
    static bool cleaned = false;
    if (cleaned) return;
    cleaned = true;

    printf("Limpando recursos...\n");

    // Desanexar e remover memória compartilhada para SensorData
    if (shared_data != NULL) shmdt(shared_data);
    shmctl(shm_id_sensors, IPC_RMID, NULL);

    // Desanexar e remover memória compartilhada para Status_trigg
    if (status_trigg != NULL) shmdt(status_trigg);
    shmctl(shm_id_triggers, IPC_RMID, NULL);

    // Fechar e remover semáforo
    if (sem_sync != NULL) {
        sem_close(sem_sync);
        sem_unlink("/sem_sync");
    }

    printf("Recursos liberados com sucesso!\n");
}


/**
 * @brief Função principal do Controlador.
 *
 * Inicializa todos os recursos necessários, como memória compartilhada, fila de mensagens
 * e semáforo. Em seguida, executa o loop principal do controlador que processa
 * mensagens recebidas do Painel de Comando e atualiza os valores dos sensores e
 * acionadores.
 *
 * Ao final, exibe um relatório sobre os acionamentos dos limitadores e
 * libera todos os recursos alocados.
 *
 * @return 0 se o programa for executado com sucesso.
 */
int main() {
    setup_signals();

    // Inicializar IPCs
    init_shared_memory();
    init_message_queue();
    init_semaphore();

    printf("Controlador inicializado. Aguardando dados...\n");

    // Executar o loop principal do controlador
    process_control();

    // Relatório dos acionadores
    printf("\n======== RELATÓRIO DOS LIMITADORES ===========\n\n");
    printf("Limite superior da velocidade %d vezes atingido.\n", cont_vel_sup);
    printf("Limite inferior da velocidade %d vezes atingido.\n", cont_vel_inf);
    printf("Limite superior do RPM %d vezes atingido.\n", cont_rpm_sup);
    printf("Limite inferior do RPM %d vezes atingido.\n", cont_rpm_inf);
    printf("Limite de temperatura %d vezes atingido.\n", cont_max_temp);
    printf("Acionamentos Totais: %d.\n", (cont_vel_sup + cont_vel_inf + cont_rpm_sup + cont_rpm_inf + cont_max_temp));
    printf("===================================================\n\n");

    // Limpar recursos antes de sair
    cleanup();

    return 0;
}
