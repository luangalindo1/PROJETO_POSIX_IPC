#include "sensor.h"
#include <wiringPi.h>
#include <time.h>

/**
 * @brief Lê a velocidade do carro em km/h a partir dos sensores Hall da roda.
 *
 * @param pin_a Pino GPIO conectado ao canal A do sensor Hall da roda.
 * @param pin_b Pino GPIO conectado ao canal B do sensor Hall da roda.
 * @param wheel_diameter Diâmetro da roda em metros.
 * @return Velocidade do carro em km/h.
 */
float sensor_read_car_speed(int pin_a, int pin_b, float wheel_diameter) {
    static int last_state_a = LOW; // Estado anterior do sensor A (LOW ou HIGH)
    static int last_state_b = LOW; // Estado anterior do sensor B (LOW ou HIGH)
    static double PI = 3.141592653589793;
    static struct timespec last_time = {0}; // Variável para armazenar o tempo anterior
    struct timespec current_time;
    if (wiringPiSetupGpio() == -1) {
        return -1; // Erro ao inicializar a biblioteca WiringPi
    }

    // Ler o estado atual dos sensores
    int current_state_a = digitalRead(pin_a);
    int current_state_b = digitalRead(pin_b);

    // Variáveis para armazenar o tempo decorrido e a velocidade de cada roda
    float speed_mps_a = 0.0;
    float speed_mps_b = 0.0;

    // Calcular a velocidade para a roda A quando ocorrer uma transição (borda de subida)
    if (current_state_a != last_state_a && current_state_a == HIGH) {
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        // Calcular o tempo decorrido desde a última transição
        float elapsed_time_s = (current_time.tv_sec - last_time.tv_sec) +
                               (current_time.tv_nsec - last_time.tv_nsec) / 1e9;

        last_time = current_time; // Atualizar o tempo de referência para a próxima leitura
        last_state_a = current_state_a; // Atualizar o estado anterior do sensor A

        // Calcular a velocidade da roda A (m/s) e converter para km/h
        speed_mps_a = (PI * wheel_diameter) / elapsed_time_s; // v = (2 * pi * r) / t = (pi * d) / t
    }

    // Calcular a velocidade para a roda B quando ocorrer uma transição (borda de subida)
    if (current_state_b != last_state_b && current_state_b == HIGH) {
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        // Calcular o tempo decorrido desde a última transição
        float elapsed_time_s = (current_time.tv_sec - last_time.tv_sec) +
                               (current_time.tv_nsec - last_time.tv_nsec) / 1e9;

        last_time = current_time; // Atualizar o tempo de referência para a próxima leitura
        last_state_b = current_state_b; // Atualizar o estado anterior do sensor B

        // Calcular a velocidade da roda B (m/s) e converter para km/h
        speed_mps_b = (PI * wheel_diameter) / elapsed_time_s;
    }

    // Se pelo menos uma roda teve uma transição, calcular a velocidade média
    if (speed_mps_a > 0.0 || speed_mps_b > 0.0) {
        // Média das velocidades das duas rodas
        float average_speed_mps = (speed_mps_a + speed_mps_b) / 2.0;
        return average_speed_mps * 3.6; // Converte a velocidade média para km/h
    }

    return 0; // Se não houve transição em nenhuma das rodas, a velocidade é zero
}

/**
 * @brief Lê a rotação por minuto (RPM) do motor a partir dos sinais do sensor Hall.
 *
 * @param pin_a Pino GPIO conectado ao canal A do sensor Hall do motor.
 * @param pin_b Pino GPIO conectado ao canal B do sensor Hall do motor.
 * @return Rotação por minuto (RPM) do motor.
 */
int sensor_read_motor_rpm(int pin_motor) {
    static int last_state_a = LOW; // LOW e HIGH definidos em <wiringPi.h>
    static struct timespec last_time = {0}; // Inicializado com zeros
    struct timespec current_time;

    if (wiringPiSetupGpio() == -1) {
        return -1; // Erro ao inicializar a biblioteca WiringPi
    }

    int current_state_a = digitalRead(pin_motor);

    if (current_state_a != last_state_a && current_state_a == HIGH) {
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        float elapsed_time_s = (current_time.tv_sec - last_time.tv_sec) +
                               (current_time.tv_nsec - last_time.tv_nsec) / 1e9;

        last_time = current_time;
        last_state_a = current_state_a;

        // Calculo do RPM: rpm = (1 / tempo_por_revolucao) * 60
        int rpm = (int)((1 / elapsed_time_s) * 60);
        return rpm;
    }

    return 0; // Se não houver transição, o RPM é zero
}
