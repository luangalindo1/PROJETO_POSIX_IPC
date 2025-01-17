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
#include <time.h>

// >>> Adicionados para GPIO e PWM <<<
#include <wiringPi.h>
#include <softPwm.h>

// Definições de chaves IPC
#define SHM_KEY_SENSORS 1234
#define SHM_KEY_TRIGGERS 4321
#define MSG_KEY 5678

// Definições de pinos para os componentes

// Direção
#define MOTOR_DIR1 17 // Direção 1 (OUT)
#define MOTOR_DIR2 18 // Direção 2 (OUT)

// Motor
#define MOTOR_POT 23 // Potência do Motor (PWM) (OUT)

// Pedais
#define FREIO_INT 24 // Intensidade do Pedal de Freio (PWM) (OUT)
#define PEDAL_AC 27  // Pedal do Acelerador (IN)
#define PEDAL_FR 22  // Pedal do Freio (IN)

// Sensores Hall
#define SENSOR_HALL_MOTOR 11      // Sensor Hall do Motor (IN)
#define SENSOR_HALL_RODA_A 5      // Sensor Hall da Roda A (IN)
#define SENSOR_HALL_RODA_B 6      // Sensor Hall da Roda B (IN)

// Faróis e luzes
#define FAROL_BAIXO 19           // Luzes de Farol Baixo (OUT)
#define FAROL_ALTO 26            // Luzes de Farol Alto (OUT)
#define LUZ_FREIO 25             // Luzes de Freio (OUT)
#define LUZ_SETA_ESQ 8           // Luz da Seta Esquerda (OUT)
#define LUZ_SETA_DIR 7           // Luz da Seta Direita (OUT)
#define LUZ_TEMP_MOTOR 12        // Luz de Alerta da Temperatura do Motor (OUT)

// Comandos
#define COMANDO_FAROL_BAIXO 16   // Comando de Ligar/Desligar Farol (IN)
#define COMANDO_FAROL_ALTO 1     // Comando de Ligar/Desligar Farol Alto (IN)
#define COMANDO_SETA_ESQ 20      // Comando de Ligar/Desligar Seta Esquerda (IN)
#define COMANDO_SETA_DIR 21      // Comando de Ligar/Desligar Seta Direita (IN)

/*// Cruise Control
#define CC_RES 13                // Comando de Cruise Control (IN)
#define CC_CANCEL 0*/              // Comando de Cancelar Cruise Control (IN)

// Definições de constantes para cálculo de temperatura
#define FATOR_ACELERACAO 0.1 
#define FATOR_RESFRIAMENTO_AR 0.05
#define MAX_TEMP_MOTOR 140
#define BASE_TEMP 80

// Estrutura para os dados dos sensores
typedef struct {
    float velocidade; 
    float rpm;          
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

// Variáveis para PWM e Contadores
static int motorDuty = 0;   // Duty cycle motor (0-10)
static int freioDuty = 0;   // Duty cycle freio (0-10)

// Tempo de referência
struct timespec ultimoTempoMotor;
struct timespec ultimoTempoRoda_a;
struct timespec ultimoTempoRoda_b;

// Contadores para sensor Hall (RPM, velocidade)
static volatile unsigned long motorPulsos = 0;  
static volatile unsigned long rodaPulsos_a = 0;
static volatile unsigned long rodaPulsos_b = 0;   

// Contadores para relatório de acionamentos
int cont_vel_sup = 0;
int cont_vel_inf = 0;
int cont_rpm_sup = 0;
int cont_rpm_inf = 0;
int cont_max_temp = 0;


/**
 * @brief Função callback para o sensor Hall do motor.
 *
 * Incrementa o contador de pulsos do motor.
 */
void motor_hall_callback(void) {
    motorPulsos++;
}
/**
 * @brief Função callback para o sensor Hall da roda A.
 *
 * Incrementa o contador de pulsos da roda A.
 */
void roda_a_hall_callback(void) {
    rodaPulsos_a++;
}
/**
 * @brief Fun o callback para o sensor Hall da roda B.
 *
 * Incrementa o contador de pulsos da roda B.
 */
void roda_b_hall_callback(void) {
    rodaPulsos_b++;
}

/**
 * @brief Função callback para tratar sinais recebidos pelo programa.
 * 
 * Trata os sinais SIGUSR1, SIGUSR2 e SIGINT.
 * 
 * Se o sinal for SIGUSR1, pausa o loop principal do programa.
 * Se o sinal for SIGUSR2 ou SIGINT, envia uma mensagem "Encerrar" para o
 * Painel de Comando e sinaliza para encerrar o programa.
 */
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
    } else if (signal == SIGINT) {
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

/**
 * @brief Instala os handlers para os sinais SIGUSR1, SIGUSR2 e SIGINT.
 *
 * SIGUSR1: Pausa o loop principal do programa. O programa pode ser
 *          retomado com um sinal SIGUSR1.
 * SIGUSR2: Encerra o programa. O painel de comando também recebe uma
 *          mensagem de encerramento.
 * SIGINT: Encerra o programa. O painel de comando também recebe uma
 *         mensagem de encerramento.
 */
void setup_signals() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Erro ao ativar handler SIGUSR1");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Erro ao ativar handler SIGUSR2");
        exit(EXIT_FAILURE);
    } 
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Erro ao ativar handler SIGINT");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Inicializa memórias compartilhadas para sensores e acionadores.
 *
 * Cria memória compartilhada para SensorData e Status_trigg, associa-as e
 * inicializa os campos com valores padrão.
 *
 * @return Nada.
 */
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

    printf("============= Memórias compartilhadas inicializadas. ===============\n");
}

/**
 * @brief Inicializa fila de mensagens para comunicação com o painel
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
    
    // Limpar mensagens residuais
    Message msg;
    while (msgrcv(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0, IPC_NOWAIT) > 0) {}
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
 *        do trabalho 1.
 *
 * @param velocidade A velocidade atual do veículo em km/h
 * @param rpm O valor do RPM do motor
 * @return A temperatura do motor em graus Celsius
 */
float calculate_engine_temp(float velocidade, float rpm) {
    float temp_rise = (rpm / 10.0) * FATOR_ACELERACAO;
    float cooling_effect = velocidade * FATOR_RESFRIAMENTO_AR;
    float temp = BASE_TEMP + temp_rise - cooling_effect;
    return (float)fmin(MAX_TEMP_MOTOR, temp);
}

/**
 * Calcula o valor do RPM do motor, baseado em uma fórmula empírica.
 *
 * Essa fórmula utiliza a quantidade de pulsos do motor nos últimos segundos
 * e aplica uma constante de conversão para obter o valor do RPM.
 *
 * A cada chamada, o valor do RPM é calculado e o contador de pulsos é
 * resetado.
 *
 * @return O valor do RPM do motor.
 */
float motor_rpm() {
    float rpm = 0.0;
    
    // Debug
    //printf("Motor pulsos pré calculo RPM: %ld\n", motorPulsos);
    
    // Constantes empregadas no cálculo do RPM
    const int RPM_CONST1 = 2285;
    const int RPM_CONST2 = 800;
    const int PULSE_CONST1 = 76;
    const int PULSE_CONST2 = 26;
    const float EMP_CONST = ((RPM_CONST1 / PULSE_CONST1) + (RPM_CONST2 / PULSE_CONST2)) / 2;
    
    rpm = motorPulsos * EMP_CONST;
    
    // Reset dos pulsos e atualização do tempo
    motorPulsos = 0;
    
    // Debug
    //printf("Motor pulsos post calculo RPM: %ld\n", motorPulsos);

    return rpm;
}

/**
 * Calcula a velocidade média com base nos pulsos dos sensores Hall das rodas.
 *
 * A função utiliza contadores de pulsos de duas rodas (A e B) para 
 * calcular a velocidade individual de cada roda com uma constante 
 * empiricamente definida. A média das velocidades das duas rodas é 
 * então calculada para dar a velocidade final do veículo.
 *
 * Após o cálculo, os contadores de pulsos são resetados.
 *
 * @return A velocidade média do veículo.
 */
float velocidade() {
    float velocidade_media = 0.0;
    float velocidade_a = 0.0;
    float velocidade_b = 0.0;
    
    // Debug
    //printf("roda_a pulsos pré calculo  %ld\n", rodaPulsos_a);
    //printf("roda_b pulsos pré calculo  %ld\n", rodaPulsos_b);

    // Constantes empregadas no cálculo da velocidade
    const int VEL_CONST1 = 144;
    const int PULSE_CONST1 = 20;
    const int PULSE_CONST2 = 21;
    const float EMP_CONST = VEL_CONST1 / ((PULSE_CONST1 + PULSE_CONST2) / 2);
    
    // Cálculo empirico
    velocidade_a = rodaPulsos_a * EMP_CONST;
    velocidade_b = rodaPulsos_b * EMP_CONST;

    velocidade_media = ((velocidade_a + velocidade_b) / 2.0);

    // Reset dos pulsos e atualização do tempo
    rodaPulsos_a = 0;
    rodaPulsos_b = 0;

    // Debug
    //printf("roda_a pulsos post calculo  %ld\n", rodaPulsos_a);
    //printf("roda_b pulsos post calculo  %ld\n", rodaPulsos_b);

    return velocidade_media;
}

/**
 * @brief Seta a direção do motor.
 *
 * A função define a direção do motor com base na entrada
 * recebida. Se a entrada for diferente de 'D', 'R', 'B'
 * ou 'N', a função define a direção como neutro.
 *
 * @param direction Direção do motor 
 * ('D' = frente (drive), 'R' = ré, 
 *  'B' = freio (brake), 'N' = neutro)
 */
void motor_set_direction(char direction) {
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

/**
 * @brief Inicializa um pino GPIO.
 *
 * A função define o pino como saída (OUTPUT) ou
 * entrada (INPUT). Se o pino for definido como entrada,
 * a função define como sem resistor de pull-up ou pull-down.
 *
 * @param pin Número do pino GPIO a ser inicializado.
 * @param direction Direção do pino (OUTPUT ou INPUT).
 */
void gpio_pin_setup(int pin, int direction) {
    if (direction == OUTPUT) {
        pinMode(pin, OUTPUT);
    } else if (direction == INPUT) {
        pinMode(pin, INPUT);
        // Sem resistor de pull-up ou pull-down por padrão
        pullUpDnControl(pin, PUD_OFF); 
    }
}

/**
 * @brief Inicializa GPIO e configura pinos.
 *
 * Inicializa o WiringPi no modo BCM, configura pinos
 * de direção do motor, pedais, PWM do motor e do freio,
 * faróis, setas, luzes e sensores Hall e configura
 * interrupções para os sensores Hall.
 *
 * @return Nenhum.
 */
void init_gpio() {
    // Inicializar WiringPi (modo BCM)
    if (wiringPiSetupGpio() < 0) {
        fprintf(stderr, "Erro ao inicializar WiringPi\n");
        exit(EXIT_FAILURE);
    }

    // Configurar pinos de direção do motor
    gpio_pin_setup(MOTOR_DIR1, OUTPUT);
    gpio_pin_setup(MOTOR_DIR2, OUTPUT);

    // Configurar pinos dos pedais
    gpio_pin_setup(PEDAL_AC, INPUT);
    gpio_pin_setup(PEDAL_FR, INPUT);

    // Configurar PWM do motor e do freio:
    // Para garantir que o PWM funcione em 1 kHz,
    // precisamos de um intervalo de 1 ms e
    // uma resolução de 11 níveis.
    // Configurando os pinos como saída
    //gpio_pin_setup(MOTOR_POT, OUTPUT);
    //gpio_pin_setup(FREIO_INT, OUTPUT);
    if (softPwmCreate(MOTOR_POT, 0, 10) != 0) {
        fprintf(stderr, "Erro ao criar PWM para MOTOR_POT\n");
        exit(EXIT_FAILURE);
    }
    if (softPwmCreate(FREIO_INT, 0, 10) != 0) {
        fprintf(stderr, "Erro ao criar PWM para FREIO_INT\n");
        exit(EXIT_FAILURE);
    }

    // Faróis / Seta (saídas digitais)
    gpio_pin_setup(FAROL_BAIXO, OUTPUT);
    gpio_pin_setup(FAROL_ALTO, OUTPUT);
    gpio_pin_setup(LUZ_SETA_ESQ, OUTPUT);
    gpio_pin_setup(LUZ_SETA_DIR, OUTPUT);

    // Faróis (entradas digitais)
    gpio_pin_setup(COMANDO_FAROL_BAIXO, INPUT);
    gpio_pin_setup(COMANDO_FAROL_ALTO, INPUT);
    gpio_pin_setup(COMANDO_SETA_ESQ, INPUT);
    gpio_pin_setup(COMANDO_SETA_DIR, INPUT);

    // Luzes (saídas digitais)
    gpio_pin_setup(LUZ_FREIO, OUTPUT);
    gpio_pin_setup(LUZ_TEMP_MOTOR, OUTPUT);
    
    // Sensores Hall (entradas)
    gpio_pin_setup(SENSOR_HALL_MOTOR, INPUT);
    gpio_pin_setup(SENSOR_HALL_RODA_A, INPUT);
    gpio_pin_setup(SENSOR_HALL_RODA_B, INPUT);
    
    // Configurar interrupções para sensor Hall
    if (wiringPiISR(SENSOR_HALL_MOTOR, INT_EDGE_RISING, &motor_hall_callback) < 0) {
        fprintf(stderr, "Erro ao configurar interrupção para SENSOR_HALL_MOTOR\n");
        exit(EXIT_FAILURE);
    }
    if (wiringPiISR(SENSOR_HALL_RODA_A, INT_EDGE_RISING, &roda_a_hall_callback) < 0) {
        fprintf(stderr, "Erro ao configurar interrupção para SENSOR_HALL_RODA_A\n");
        exit(EXIT_FAILURE);
    }
    if (wiringPiISR(SENSOR_HALL_RODA_B, INT_EDGE_RISING, &roda_b_hall_callback) < 0) {
        fprintf(stderr, "Erro ao configurar interrupção para SENSOR_HALL_RODA_B\n");
        exit(EXIT_FAILURE);
    }

    printf("========== GPIO inicializada. ==========\n");
}


/**
 * @brief Thread para piscar seta esquerda
 *
 * A thread PiscaSetaEsq é responsável por piscar a seta esquerda
 * quando o status_trigg->seta_esq estiver ativo. Ela espera por
 * um sinal de sincronização, consulta o status da seta esquerda e
 * executa a ação de piscar ou desligar a seta. Se a seta estiver
 * ativa, a thread dorme por 1 segundo, liga e desliga a seta,
 * e volta a dormir por 1 segundo. Se a seta estiver desativada,
 * a thread garante que a seta esteja desligada e dorme por 200
 * milissegundos.
 * 
 * @param arg Argumento da thread (não utilizado neste caso)
 * @return NULL
 */
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

/**
 * @brief Thread para piscar seta direita
 *
 * A thread PiscaSetaDir é responsável por piscar a seta direita
 * quando o status_trigg->seta_dir estiver ativo. Ela espera por
 * um sinal de sincronização, consulta o status da seta direita e
 * executa a ação de piscar ou desligar a seta. Se a seta estiver
 * ativa, a thread dorme por 1 segundo, liga e desliga a seta,
 * e volta a dormir por 1 segundo. Se a seta estiver desativada,
 * a thread garante que a seta esteja desligada e dorme por 200
 * milissegundos.
 *
 * @param arg Argumento da thread (não utilizado neste caso)
 * @return NULL
 */
void *threadPiscaSetaDir(void *arg) {
    (void)arg;

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

/**
 * @brief Thread para ler comandos do painel de comando.
 *
 * A thread ThreadComandosDash é responsável por ler os comandos do painel
 * de comando e executar ações correspondentes. Ela lê constantemente os
 * pedais do acelerador e freio, e executa ações de aceleração ou frenagem
 * dependendo do estado dos pedais. Além disso, a thread lê os comandos de
 * faróis e setas e atualiza o status_trigg com os novos valores.
 *
 * @param arg Argumento da thread (não utilizado neste caso)
 * @return NULL
 */
void *threadComandosDash(void *arg) {
    (void)arg;

    while (running) {
        // Leitura dos pedais
        if (digitalRead(PEDAL_AC)) {
            freioDuty = 0;
            softPwmWrite(FREIO_INT, freioDuty);
            digitalWrite(LUZ_FREIO, LOW);
            motor_set_direction('D');
            motorDuty = (motorDuty < 10) ? motorDuty + 1 : 10;
            softPwmWrite(MOTOR_POT, motorDuty);
        } else if (digitalRead(PEDAL_FR)) {
            motorDuty = 0;
            softPwmWrite(MOTOR_POT, motorDuty);
            digitalWrite(LUZ_FREIO, HIGH);
            motor_set_direction('B');
            freioDuty = (freioDuty < 10) ? freioDuty + 1 : 10;
            softPwmWrite(FREIO_INT, freioDuty);
        }
        if (digitalRead(COMANDO_FAROL_BAIXO)) {
            sem_wait(sem_sync);
            status_trigg->farol_baixo = !status_trigg->farol_baixo;
            digitalWrite(FAROL_BAIXO, status_trigg->farol_baixo ? HIGH : LOW);
            sem_post(sem_sync);
        } 
        if (digitalRead(COMANDO_FAROL_ALTO)) {
            sem_wait(sem_sync);
            status_trigg->farol_alto = !status_trigg->farol_alto;
            digitalWrite(FAROL_ALTO, status_trigg->farol_alto ? HIGH : LOW);
            sem_post(sem_sync);
        } 
        if (digitalRead(COMANDO_SETA_ESQ)) {
            sem_wait(sem_sync);
            status_trigg->seta_esq = !status_trigg->seta_esq;
            sem_post(sem_sync);
        }
        if (digitalRead(COMANDO_SETA_DIR)) {
            sem_wait(sem_sync);
            status_trigg->seta_dir = !status_trigg->seta_dir;
            sem_post(sem_sync);
        }
        usleep(50000); // Intervalo para evitar polling agressivo
    }

    return NULL;
}

/**
 * @brief Executa o controle principal do sistema.
 *
 * A função process_control é responsável por criar e gerenciar threads
 * para piscar setas e ler comandos do painel. Ela executa o loop principal
 * que monitora dados de sensores como velocidade, RPM e temperatura,
 * aplicando regras de segurança e limites. A função também processa
 * comandos recebidos do painel de controle, permitindo a interação
 * com diversos acionadores, como setas, faróis, e pedais do veículo.
 * 
 * A função garante a correta sincronização de dados compartilhados
 * utilizando semáforos e gerencia o estado dos acionadores do veículo.
 */
void process_control() {
    // Criar threads para piscar setas
    pthread_t th_esq, th_dir, th_comandos;
    if (pthread_create(&th_esq, NULL, threadPiscaSetaEsq, NULL) != 0) {
        perror("Erro ao criar thread para piscar seta esquerda");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&th_dir, NULL, threadPiscaSetaDir, NULL) != 0) {
        perror("Erro ao criar thread para piscar seta direita");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&th_comandos, NULL, threadComandosDash, NULL) != 0) {
        perror("Erro ao criar thread comandos do dashboard");
        exit(EXIT_FAILURE);
    }

    // Loop principal
    while (running) {
        float aux_vel, aux_temp, aux_rpm;

        // Obter dados da memória
        sem_wait(sem_sync);
        aux_vel = shared_data->velocidade;
        aux_rpm = shared_data->rpm;
        aux_temp = shared_data->temperatura;
        sem_post(sem_sync);

        // Mostrar dados
        printf("\n===== Dados dos Sensores =====\n");
        printf("Velocidade: %.2f km/h\n", aux_vel);
        printf("RPM: %.2f\n", aux_rpm);
        printf("Temperatura: %.2f ºC\n", aux_temp);

        // Atualizar velocidade e RPM
        clock_gettime(CLOCK_MONOTONIC, &ultimoTempoRoda_a);
        clock_gettime(CLOCK_MONOTONIC, &ultimoTempoRoda_b);
        clock_gettime(CLOCK_MONOTONIC, &ultimoTempoMotor);
        aux_vel = velocidade();
        aux_rpm = motor_rpm();

        // Regras de limite
        if (aux_vel > 200.0) {
            motorDuty = (motorDuty > 0) ? motorDuty - 1 : 0;
            softPwmWrite(MOTOR_POT, motorDuty);
            cont_vel_sup++;
        } else if (aux_vel < 20.0 && aux_vel > 0.0) {
            motorDuty = (motorDuty < 10) ? motorDuty + 1 : 10;
            softPwmWrite(MOTOR_POT, motorDuty); 
            cont_vel_inf++;
        }
        if (aux_rpm > 7000) {
            motorDuty = (motorDuty > 0) ? motorDuty - 1 : 0;
            softPwmWrite(MOTOR_POT, motorDuty);
            cont_rpm_sup++;
        } else if (aux_rpm < 780) {
            motorDuty = 0;
            softPwmWrite(MOTOR_POT, motorDuty); 
            cont_rpm_inf++;
            printf("\n========= O motor apagou =========\n");
            raise(SIGUSR2);
        }
        if (aux_temp >= MAX_TEMP_MOTOR) {
            printf("\n========= ALERTA DE TEMPERATURA =========\n");
            cont_max_temp++;
            motorDuty = (motorDuty > 0) ? motorDuty - 1 : 0;
            softPwmWrite(MOTOR_POT, motorDuty);
            digitalWrite(LUZ_TEMP_MOTOR, HIGH);
        } else {
            digitalWrite(LUZ_TEMP_MOTOR, LOW);
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
            printf("\n ===== Comando recebido do Painel: %s =====\n", msg.command);

            // Processar comando
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
            } else if (strcmp(msg.command, "Ligar Pisca-Alerta") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_esq= true;
                status_trigg->seta_dir = true;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Pisca-Alerta") == 0) {
                sem_wait(sem_sync);
                status_trigg->seta_esq = false;
                status_trigg->seta_dir = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Ligar Farol Baixo") == 0) {
                digitalWrite(FAROL_BAIXO, HIGH);
                sem_wait(sem_sync);
                status_trigg->farol_baixo = true;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Farol Baixo") == 0) {
                digitalWrite(FAROL_BAIXO, LOW);
                sem_wait(sem_sync);
                status_trigg->farol_baixo = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Ligar Farol Alto") == 0) {
                digitalWrite(FAROL_ALTO, HIGH);
                sem_wait(sem_sync);
                status_trigg->farol_alto = true;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Farol Alto") == 0) {
                digitalWrite(FAROL_ALTO, LOW);
                sem_wait(sem_sync);
                status_trigg->farol_alto = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Desligar Farol") == 0) {
                digitalWrite(FAROL_BAIXO, LOW);
                digitalWrite(FAROL_ALTO, LOW);
                sem_wait(sem_sync);
                status_trigg->farol_baixo = false;
                status_trigg->farol_alto = false;
                sem_post(sem_sync);
            } else if (strcmp(msg.command, "Acionar Pedal do Acelerador") == 0) {
                // Desabilitar freio
                freioDuty = 0;
                softPwmWrite(FREIO_INT, freioDuty);
                digitalWrite(LUZ_FREIO, LOW);

                // Ajustar direção para frente
                motor_set_direction('D');

                // Aumentar duty cycle do motor
                motorDuty = (motorDuty < 10) ? motorDuty + 1 : 10;
                softPwmWrite(MOTOR_POT, motorDuty);

            } else if (strcmp(msg.command, "Acionar Pedal do Freio") == 0) {
                // Desabilitar motor
                motorDuty = 0;
                softPwmWrite(MOTOR_POT, motorDuty);
                digitalWrite(LUZ_FREIO, HIGH);
                
                // Setar motor em 'B' (freio ativo)
                motor_set_direction('B');

                // Aumentar duty cycle do freio
                freioDuty = (freioDuty < 10) ? freioDuty + 1 : 10;
                softPwmWrite(FREIO_INT, freioDuty);

            } else if (strcmp(msg.command, "Encerrar") == 0) {
                raise(SIGUSR2);
            }
        }
        
        // Simular tempo de processamento
        sleep(2);
    }
}


/**
 * @brief Limpa recursos antes de sair do programa.
 *
 * Limpa todos os recursos alocados durante a execução do programa.
 * Isso inclui:
 *  - Zerar PWM do motor e do freio;
 *  - Desligar faróis e setas;
 *  - Desanexar e remover memória compartilhada;
 *  - Fechar semáforo.
 *
 * @return Nenhum.
 */
void cleanup() {
    static bool cleaned = false;
    if (cleaned) return; 
    cleaned = true;

    printf("======== Limpando recursos...========\n");

    // Zerar PWM
    softPwmWrite(MOTOR_POT, 0);
    softPwmWrite(FREIO_INT, 0);
    motor_set_direction('N');

    // Desligar faróis e setas
    digitalWrite(FAROL_BAIXO, LOW);
    digitalWrite(FAROL_ALTO, LOW);
    digitalWrite(LUZ_SETA_ESQ, LOW);
    digitalWrite(LUZ_SETA_DIR, LOW);
    digitalWrite(LUZ_TEMP_MOTOR, LOW);
    digitalWrite(LUZ_FREIO, LOW);

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

    printf("======== Recursos liberados com sucesso!========\n");
}

/**
 * @brief Ponto de entrada do programa.
 *
 * Inicializa todos os recursos necessários e executa o loop principal do
 * programa. Ao final, limpa todos os recursos e exibe um relatório sobre os
 * acionamentos dos limitadores do programa.
 *
 * @return 0 se o programa for executado com sucesso.
 */
int main() {
    setup_signals();

    // Inicializar IPC
    init_shared_memory();
    init_message_queue();
    init_semaphore();

    // Inicializar GPIO e PWM
    init_gpio();

    printf("======== Controlador inicializado. Aguardando dados... ========\n");

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