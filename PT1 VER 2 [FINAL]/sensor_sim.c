#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#define SHM_KEY_SENSORS 1234      // Chave da memória dos sensores
#define NUM_SENSORS 3             // Quantidade de sensores (velocidade, RPM, temperatura)

// Definições de constantes da função de cálculo da temperatura do motor
#define FACTOR_ACELERACAO 0.1 
#define FATOR_RESFRIAMENTO_AR 0.05
#define MAX_TEMP_MOTOR 140
#define BASE_TEMP 80


// Estrutura para os dados dos sensores
typedef struct {
    float velocidade;   // Velocidade do carro (km/h)
    int rpm;            // Rotação do motor (RPM)
    float temperatura;  // Temperatura do motor (ºC)
} SensorData;

// Ponteiro para a memória compartilhada
SensorData *shared_data;

// Semáforo para sincronização
sem_t *sem_sync;

/**
 * @brief Gera um valor flutuante aleatório entre @p min e @p max.
 *
 * @param min Valor mínimo do intervalo.
 * @param max Valor máximo do intervalo.
 * @return Valor flutuante aleatório entre @p min e @p max.
 */
float random_float(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
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
 * @brief Simula o funcionamento de um sensor de velocidade.
 *
 * Esta função executa continuamente em uma thread separada,
 * gerando valores aleatórios de velocidade entre 0 e 200 km/h.
 * Os valores são armazenados na memória compartilhada, garantindo
 * exclusão mútua através de um semáforo. A velocidade atualizada
 * é então exibida no console. A função simula um atraso entre
 * leituras para imitar o comportamento de um sensor real.
 *
 * @param arg Argumento para a thread (não utilizado).
 * @return NULL
 */
void *sensor_velocidade(void *arg) {
    (void)arg; // Silenciar warning de parâmetro não utilizado

    while (1) {
        // valor de teste
        //float velocidade = 100.0;

        float velocidade = random_float(0, 200); // Velocidade entre 0 e 200 km/h

        sem_wait(sem_sync); // Entrar na seção crítica
        shared_data->velocidade = velocidade;
        sem_post(sem_sync); // Sair da seção crítica

        printf("[Sensor Velocidade] Atualizado: %.0f km/h\n", velocidade);
        sleep(1); // Simular tempo entre leituras
    }
    return NULL;
}


/**
 * @brief Simula o funcionamento de um sensor de RPM.
 *
 * Esta função executa continuamente em uma thread separada,
 * gerando valores aleatórios de RPM entre 500 e 8000.
 * Os valores são armazenados na memória compartilhada, garantindo
 * exclusão mútua através de um semáforo. O valor atualizado
 * é então exibido no console. A função simula um atraso entre
 * leituras para imitar o comportamento de um sensor real.
 *
 * @param arg Argumento para a thread (não utilizado).
 * @return NULL
 */
void *sensor_rpm(void *arg) {
    (void)arg;

    while (1) {
        // valor de teste
        //int rpm = 3000;
        int rpm = (int)random_float(500, 8000); // RPM entre 500 e 8000

        sem_wait(sem_sync); 
        shared_data->rpm = rpm;
        sem_post(sem_sync);

        printf("[Sensor RPM] Atualizado: %d RPM\n", rpm);
        sleep(1); // Simular tempo entre leituras
    }
    return NULL;
}


/**
 * @brief Simula o funcionamento de um sensor de temperatura.
 *
 * Esta função executa continuamente em uma thread separada,
 * gerando valores aleatórios de temperatura entre 20ºC e 120ºC,
 * ou com a função calculate_engine_temp().
 * Os valores são armazenados na memória compartilhada, garantindo
 * exclusão mútua através de um semáforo. A temperatura atualizada
 * é então exibida no console. A função simula um atraso entre
 * leituras para imitar o comportamento de um sensor real.
 * 
 * @note Esta função utiliza por default a função calculate_engine_temp() 
 * para calcular a temperatura do motor.
 *
 * @param arg Argumento para a thread (não utilizado).
 * @return NULL
 */
void *sensor_temperatura(void *arg) {
    (void)arg; 

    while (1) {
        float temperatura, velocidade;
        int rpm;
        
        // valor de teste
        //temperatura = 90.0;
        
        //temperatura = random_float(20.0, 120.0); // Temperatura entre 20ºC e 120ºC

        sem_wait(sem_sync); 
        rpm = shared_data->rpm;
        velocidade = shared_data->velocidade;
        sem_post(sem_sync); 
        
        temperatura = calculate_engine_temp(velocidade, rpm);

        sem_wait(sem_sync); 
        shared_data->temperatura = temperatura;
        sem_post(sem_sync);

        printf("[Sensor Temperatura] Atualizado: %.2f ºC\n", temperatura);
        sleep(1); // Simular tempo entre leituras
    }
    return NULL;
}


/**
 * @brief Cria e inicializa a memória compartilhada para os dados dos sensores.
 *
 * Esta função cria uma memória compartilhada para armazenar os dados dos
 * sensores (velocidade, RPM e temperatura) e a associa ao espaço de endereçamento
 * do processo.
 *
 * @note Esta função utiliza a chave SHM_KEY_SENSORS para criar a memória
 * compartilhada.
 *
 * @return Nada.
 */
void init_shared_memory() {
    int shm_id = shmget(SHM_KEY_SENSORS, sizeof(SensorData), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Erro ao criar memória compartilhada");
        exit(EXIT_FAILURE);
    }

    shared_data = (SensorData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Erro ao associar memória compartilhada");
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief Inicializa o semáforo para sincronizar o acesso
 *        à memória compartilhada.
 *
 * Cria o semáforo "/sem_sync" com valor inicial de 1.
 *
 * @return Nada.
 */
void init_semaphore() {
    sem_sync = sem_open("/sem_sync", O_CREAT, 0666, 1);
    if (sem_sync == SEM_FAILED) {
        perror("Erro ao criar semáforo");
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief Ponto de entrada do programa para simulação de sensores.
 *
 * Inicializa os recursos necessários, incluindo memória compartilhada
 * e semáforo, e cria threads para simular sensores de velocidade, RPM
 * e temperatura. Cada thread executa continuamente, atualizando os
 * valores dos sensores na memória compartilhada. O programa aguarda
 * a finalização das threads e, em seguida, libera os recursos
 * alocados antes de encerrar.
 *
 * @return 0 se o programa for executado com sucesso.
 */
int main() {
    srand(time(NULL)); // Inicializar a semente para geração de números aleatórios

    // Inicializar memória compartilhada e semáforo
    init_shared_memory();
    init_semaphore();

    // Criar threads para os sensores
    pthread_t threads[NUM_SENSORS];

    if (pthread_create(&threads[0], NULL, sensor_velocidade, NULL) != 0) {
        perror("Erro ao criar thread para sensor de velocidade");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&threads[1], NULL, sensor_rpm, NULL) != 0) {
        perror("Erro ao criar thread para sensor de RPM");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&threads[2], NULL, sensor_temperatura, NULL) != 0) {
        perror("Erro ao criar thread para sensor de temperatura");
        exit(EXIT_FAILURE);
    }

    // Aguardar que as threads (sensores) terminem
    for (int i = 0; i < NUM_SENSORS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Fechar o semáforo e desconectar a memória compartilhada
    sem_close(sem_sync);
    shmdt(shared_data);

    return 0;
}
