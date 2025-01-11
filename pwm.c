#include <wiringPi.h>
#include <softPwm.h>
#include "pwm.h"

/**
 * @brief Configura um pino GPIO para gerar sinal PWM usando SoftPWM.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @param initialValue Valor inicial do duty cycle (0-100).
 * @return 0 se configurado com sucesso, -1 se falhar.
 */
int pwm_setup(int pin, int initialValue) {
    if (wiringPiSetupGpio() == -1) {
        return -1; // Falha ao inicializar a biblioteca WiringPi
    }

    if (softPwmCreate(pin, initialValue, 100) != 0) {
        return -1; // Falha ao configurar SoftPWM
    }

    return 0;
}

/**
 * @brief Define o duty cycle de um pino configurado para PWM.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @param dutyCycle Duty cycle desejado (0-100).
 * @return 0 se configurado com sucesso, -1 se falhar.
 */
int pwm_write(int pin, int dutyCycle) {
    if (dutyCycle < 0 || dutyCycle > 100) {
        return -1; // Duty cycle fora do intervalo permitido
    }

    softPwmWrite(pin, dutyCycle);
    return 0;
}

/**
 * @brief Para o sinal PWM em um pino GPIO.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @return 0 se bem-sucedido, -1 se falhar.
 */
int pwm_stop(int pin) {
    softPwmWrite(pin, 0); // Define o duty cycle para 0
    return 0;
}
