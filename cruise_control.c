#include "cruise_control.h"
#include "sensor.h"
#include "motor_control.h"
#include "basic_def.h"
#include "gpio.h"
static bool cruise_control_enabled = FALSE;  // Flag para indicar se o cruise control está ativo
static float set_speed = 0;               // Velocidade desejada (set point)

/**
 * @brief Ativa o sistema de Cruise Control e define a velocidade alvo.
 *
 * @param speed Velocidade desejada em km/h.
 */
void cruise_control_enable(float speed) {
    cruise_control_enabled = 1;
    set_speed = speed;
    gpio_write(CC_RES, HIGH);
    gpio_write(CC_CANCEL, LOW);
    printf("Cruise control ativado. Velocidade configurada para %.0f km/h\n", set_speed);
}

/**
 * @brief Desativa o sistema de Cruise Control.
 */
void cruise_control_disable() {
    cruise_control_enabled = 0;
    gpio_write(CC_CANCEL, HIGH);
    gpio_write(CC_RES, LOW);
    printf("Cruise control desativado.\n");
}

/**
 * @brief Ajusta a velocidade do Cruise Control em um incremento ou decremento.
 *
 * @param delta Valor a ser adicionado ou subtraído da velocidade atual.
 */
void cruise_control_adjust_speed(int delta) {
    if (cruise_control_enabled) {
        set_speed += (float)delta;
        set_speed = (set_speed < 0) ? 0 : set_speed;
        set_speed = (set_speed > 200) ? 200 : set_speed;
        //printf("Velocidade ajustada para %d km/h\n", set_speed);
    } /*else {
        printf("Cruise control está desativado.\n");
    }*/
}

/**
 * @brief Loop principal do Cruise Control para ajustar a velocidade do motor.
 * Deve ser chamado periodicamente em uma thread separada.
 */
void cruise_control_loop() {

    while (cruise_control_enabled) {
        float current_speed = sensor_read_car_speed(SENSOR_HALL_RODA_A, SENSOR_HALL_RODA_B, WHEEL_DIAMETER); // Lê a velocidade atual do sensor

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
