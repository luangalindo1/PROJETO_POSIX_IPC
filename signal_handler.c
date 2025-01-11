#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "signal_handler.h"
#include "motor_control.h"
#include "pwm.h"

/**
 * @brief Tratador de sinal SIGINT (Ctrl + C) para encerramento seguro.
 *
 * @param signo Número do sinal recebido.
 */
void handle_sigint(int signo) {
    if (signo == SIGINT) {
        printf("\nEncerrando o programa de forma segura...\n");

        // Desligar todos os periféricos
        motor_stop(17, 18, 23); // GPIOs do motor: 17, 18, 23

        // Finalizar o programa
        exit(0);
    }
}

/**
 * @brief Configura o tratador de sinal para SIGINT.
 */
void setup_signal_handler() {
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Erro ao configurar o tratador de sinal SIGINT");
        exit(1);
    }
}
