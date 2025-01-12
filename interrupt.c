#include "interrupt.h"
#include <wiringPi.h>

/**
 * @brief Configura uma interrupção em um pino GPIO.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @param edge Tipo de borda para disparar a interrupção:
 *        - INT_EDGE_FALLING: Borda de descida.
 *        - INT_EDGE_RISING: Borda de subida.
 *        - INT_EDGE_BOTH: Ambos os sentidos.
 * @param callback Ponteiro para a função que será chamada quando a interrupção for disparada.
 * @return 0 se configurado com sucesso, -1 se falhar.
 */
int interrupt_setup(int pin, int edge, void (*callback)(void)) {
    if (wiringPiSetupGpio() == -1) {
        return -1; // Falha ao inicializar a biblioteca WiringPi
    }

    if (wiringPiISR(pin, edge, callback) < 0) {
        return -1; // Falha ao configurar a interrupção
    }

    return 0;
}

/**
 * @brief Desabilita uma interrupção associada a um pino GPIO.
 *
 * @param pin O número do pino GPIO no esquema BCM.
 * @return 0 se desabilitado com sucesso, -1 se falhar.
 */
int interrupt_disable(int pin) {
    pinMode(pin, INPUT); // Reconfigura o pino como entrada simples
    return 0;
}
