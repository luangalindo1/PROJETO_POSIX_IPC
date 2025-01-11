#include <wiringPi.h>
#include "pwm.h"
#include "motor_control.h"

/**
 * @brief Configura os pinos de direção e potência do motor.
 *
 * @param dir1 Pino GPIO para Direção 1.
 * @param dir2 Pino GPIO para Direção 2.
 * @param pwm Pino GPIO para controle de potência (PWM).
 * @return 0 se configurado com sucesso, -1 se falhar.
 */
int motor_setup(int dir1, int dir2, int pwm) {
    pinMode(dir1, OUTPUT);
    pinMode(dir2, OUTPUT);

    if (pwm_setup(pwm, 0) < 0) {
        return -1; // Falha ao configurar o pino PWM
    }
    return 0;
}

/**
 * @brief Define a direção do motor.
 *
 * @param dir1 Pino GPIO para Direção 1.
 * @param dir2 Pino GPIO para Direção 2.
 * @param direction Direção desejada:
 *        - MOTOR_FORWARD: Andar para frente.
 *        - MOTOR_REVERSE: Marcha ré.
 *        - MOTOR_BRAKE: Freio.
 *        - MOTOR_FREE: Motor livre.
 */
void motor_set_direction(int dir1, int dir2, int direction) {
    switch (direction) {
        case MOTOR_FORWARD:
            digitalWrite(dir1, HIGH);
            digitalWrite(dir2, LOW);
            break;
        case MOTOR_REVERSE:
            digitalWrite(dir1, LOW);
            digitalWrite(dir2, HIGH);
            break;
        case MOTOR_BRAKE:
            digitalWrite(dir1, HIGH);
            digitalWrite(dir2, HIGH);
            break;
        case MOTOR_FREE:
        default:
            digitalWrite(dir1, LOW);
            digitalWrite(dir2, LOW);
            break;
    }
}

/**
 * @brief Define a potência do motor via PWM.
 *
 * @param pwm Pino GPIO para controle de potência (PWM).
 * @param speed Duty cycle desejado (0-100).
 */
void motor_set_speed(int pwm, int speed) {
    pwm_write(pwm, speed);
}

/**
 * @brief Para o motor.
 *
 * @param dir1 Pino GPIO para Direção 1.
 * @param dir2 Pino GPIO para Direção 2.
 * @param pwm Pino GPIO para controle de potência (PWM).
 */
void motor_stop(int dir1, int dir2, int pwm) {
    motor_set_direction(dir1, dir2, MOTOR_FREE);
    motor_set_speed(pwm, 0);
}
