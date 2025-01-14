/**
 * sensor_sim.c
 *
 * Versão corrigida para silenciar warnings de parâmetros não utilizados e
 * linkagem correta com a biblioteca matemática.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

// Estrutura para os dados dos sensores
typedef struct {
    float velocidade; 
    int rpm;          
    float temperatura;
} SensorData;

// Função para calcular a temperatura do motor
float calculate_engine_temp(float velocidade, int rpm) {
    float temp_rise = (rpm / 10.0) * 0.1; // FACTOR_ACELERACAO
    float cooling_effect = velocidade * 0.05; // FATOR_RESFRIAMENTO_AR
    float temp = 80.0 + temp_rise - cooling_effect; // BASE_TEMP
    return fmin(140.0, temp); // MAX_TEMP_MOTOR
}

// Função para simular velocidade
void *sensor_velocidade(void *arg) {
    (void)arg; // Silencia warning de parâmetro não utilizado
    SensorData *data = (SensorData *)arg;
    while (1) {
        // Simulação de atualização de velocidade
        data->velocidade += 5.0;
        if (data->velocidade > 200.0)
            data->velocidade = 0.0;
        sleep(1);
    }
    return NULL;
}

// Função para simular RPM
void *sensor_rpm(void *arg) {
    (void)arg; // Silencia warning de parâmetro não utilizado
    SensorData *data = (SensorData *)arg;
    while (1) {
        // Simulação de atualização de RPM
        data->rpm += 100;
        if (data->rpm > 8000)
            data->rpm = 800;
        sleep(1);
    }
    return NULL;
}

// Função para simular temperatura
void *sensor_temperatura(void *arg) {
    (void)arg; // Silencia warning de parâmetro não utilizado
    SensorData *data = (SensorData *)arg;
    while (1) {
        // Simulação de atualização de temperatura
        data->temperatura = calculate_engine_temp(data->velocidade, data->rpm);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    (void)argc; // Silencia warning de parâmetro não utilizado
    (void)argv; // Silencia warning de parâmetro não utilizado

    SensorData data;
    data.velocidade = 0.0;
    data.rpm = 800;
    data.temperatura = calculate_engine_temp(data.velocidade, data.rpm);

    pthread_t thread_velocidade, thread_rpm, thread_temperatura;

    if (pthread_create(&thread_velocidade, NULL, sensor_velocidade, &data) != 0) {
        perror("Erro ao criar thread de velocidade");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_rpm, NULL, sensor_rpm, &data) != 0) {
        perror("Erro ao criar thread de RPM");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_temperatura, NULL, sensor_temperatura, &data) != 0) {
        perror("Erro ao criar thread de temperatura");
        exit(EXIT_FAILURE);
    }

    // Exibir os dados em loop
    while (1) {
        printf("\n===== Dados dos Sensores =====\n");
        printf("Velocidade: %.1f km/h\n", data.velocidade);
        printf("RPM: %d\n", data.rpm);
        printf("Temperatura: %.2f ºC\n", data.temperatura);
        sleep(1);
    }

    // Nunca será alcançado, mas por completude:
    pthread_join(thread_velocidade, NULL);
    pthread_join(thread_rpm, NULL);
    pthread_join(thread_temperatura, NULL);

    return 0;
}
