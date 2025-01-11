#include <wiringPi.h>
#include "gpio.h"

/**
 * @brief Configura um pino GPIO como entrada ou saída.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @param direction Direção do pino: 1 para saída, 0 para entrada.
 * @return 0 se configurado com sucesso, -1 se falhar.
 */
int gpio_setup(int pin, int direction) {
    if (wiringPiSetupGpio() == -1) {
        return -1; // Falha ao inicializar a biblioteca
    }

    if (direction == GPIO_OUT) {
        pinMode(pin, OUTPUT);
    } else if (direction == GPIO_IN) {
        pinMode(pin, INPUT);
        pullUpDnControl(pin, PUD_OFF); // Sem resistor de pull-up ou pull-down por padrão
    } else {
        return -1; // Direção inválida
    }

    return 0;
}

/**
 * @brief Escreve um valor digital (HIGH ou LOW) em um pino de saída GPIO.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @param value Valor a ser escrito: HIGH ou LOW.
 * @return 0 se escrito com sucesso, -1 se falhar.
 */
int gpio_write(int pin, int value) {
    if (value != HIGH && value != LOW) {
        return -1; // Valor inválido
    }

    digitalWrite(pin, value);
    return 0;
}

/**
 * @brief Lê o valor digital de um pino de entrada GPIO.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @return O valor lido: HIGH ou LOW, ou -1 se falhar.
 */
int gpio_read(int pin) {
    return digitalRead(pin);
}

/**
 * @brief Configura uma interrupção em um pino GPIO.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @param edge Tipo de borda (INT_EDGE_RISING, INT_EDGE_FALLING, INT_EDGE_BOTH).
 * @param callback Função a ser chamada quando a interrupção ocorrer.
 * @return 0 se configurado com sucesso, -1 se falhar.
 */
int gpio_set_interrupt(int pin, int edge, void (*callback)(void)) {
    if (wiringPiISR(pin, edge, callback) < 0) {
        return -1; // Falha ao configurar interrupção
    }

    return 0;
}
