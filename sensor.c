#include <wiringPi.h>
#include <time.h>
#include "sensor.h"

/**
 * @brief Lê a velocidade do carro em km/h a partir dos sensores Hall da roda.
 *
 * @param pin_a Pino GPIO conectado ao canal A do sensor Hall da roda.
 * @param pin_b Pino GPIO conectado ao canal B do sensor Hall da roda.
 * @param wheel_diameter Diâmetro da roda em metros.
 * @return Velocidade do carro em km/h.
 */
float sensor_read_car_speed(int pin_a, int pin_b, float wheel_diameter) {
    static int last_state_a = LOW; // LOW e HIGH definidos em <wiringPi.h>
    static struct timespec last_time = {0}; // Inicializado com zeros
    struct timespec current_time;

    if (wiringPiSetupGpio() == -1) {
        return -1; // Erro ao inicializar a biblioteca WiringPi
    }

    int current_state_a = digitalRead(pin_a);

    if (current_state_a != last_state_a && current_state_a == HIGH) {
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        float elapsed_time_s = (current_time.tv_sec - last_time.tv_sec) +
                               (current_time.tv_nsec - last_time.tv_nsec) / 1e9;

        last_time = current_time;
        last_state_a = current_state_a;

        // Calculo da velocidade: v = (pi * d) / tempo_por_revolucao
        float speed_mps = (3.14159 * wheel_diameter) / elapsed_time_s;
        return speed_mps * 3.6; // Converte para km/h
    }

    return 0; // Se não houver transição, a velocidade é zero
}

/**
 * @brief Lê a rotação por minuto (RPM) do motor a partir dos sinais do sensor Hall.
 *
 * @param pin_a Pino GPIO conectado ao canal A do sensor Hall do motor.
 * @param pin_b Pino GPIO conectado ao canal B do sensor Hall do motor.
 * @return Rotação por minuto (RPM) do motor.
 */
int sensor_read_motor_rpm(int pin_a, int pin_b) {
    static int last_state_a = LOW; // LOW e HIGH definidos em <wiringPi.h>
    static struct timespec last_time = {0}; // Inicializado com zeros
    struct timespec current_time;

    if (wiringPiSetupGpio() == -1) {
        return -1; // Erro ao inicializar a biblioteca WiringPi
    }

    int current_state_a = digitalRead(pin_a);

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
