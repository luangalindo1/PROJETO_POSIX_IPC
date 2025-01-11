#include <stdio.h>
#include <unistd.h>
#include "pwm.h"
#include "sensor.h"
#include "motor_control.h"
#include "cruise_control.h"

static int cruise_control_enabled = 0;  // Flag para indicar se o cruise control está ativo
static int set_speed = 0;               // Velocidade desejada (set point)

/**
 * @brief Ativa o sistema de Cruise Control e define a velocidade alvo.
 *
 * @param speed Velocidade desejada em km/h.
 */
void cruise_control_enable(int speed) {
    cruise_control_enabled = 1;
    set_speed = speed;
    printf("Cruise control ativado. Velocidade configurada para %d km/h\n", set_speed);
}

/**
 * @brief Desativa o sistema de Cruise Control.
 */
void cruise_control_disable() {
    cruise_control_enabled = 0;
    printf("Cruise control desativado.\n");
}

/**
 * @brief Ajusta a velocidade do Cruise Control em um incremento ou decremento.
 *
 * @param delta Valor a ser adicionado ou subtraído da velocidade atual.
 */
void cruise_control_adjust_speed(int delta) {
    if (cruise_control_enabled) {
        set_speed += delta;
        if (set_speed < 0) {
            set_speed = 0;
        }
        printf("Velocidade ajustada para %d km/h\n", set_speed);
    } else {
        printf("Cruise control está desativado.\n");
    }
}

/**
 * @brief Loop principal do Cruise Control para ajustar a velocidade do motor.
 * Deve ser chamado periodicamente em uma thread separada.
 */
void cruise_control_loop() {
    const int pin_a = 2;               // Pino de entrada do sensor A
    const int pin_b = 3;               // Pino de entrada do sensor B
    const float wheel_diameter = 0.6; // Diâmetro da roda em metros

    while (cruise_control_enabled) {
        float current_speed = sensor_read_car_speed(pin_a, pin_b, wheel_diameter); // Lê a velocidade atual do sensor

        if (current_speed < set_speed) {
            motor_set_speed(50, set_speed); // Aumenta a potência do motor para acelerar
        } else if (current_speed > set_speed) {
            motor_set_speed(0, set_speed);  // Reduz a potência para desacelerar
        } else {
            motor_set_speed(25, set_speed); // Mantém a velocidade
        }

        usleep(500000); // Aguarda 500ms antes de ajustar novamente
    }
}
