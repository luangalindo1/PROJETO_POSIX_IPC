#include "signal_handler.h"
#include "basic_def.h"
#include "motor_control.h"
/**
 * @brief Tratador de sinal SIGINT (Ctrl + C) para encerramento seguro.
 *
 * @param signo Número do sinal recebido.
 */
void handle_sigint(int signal_read) {
    if (signal_read == SIGINT) {
        printf("\nEncerrando o programa de forma segura...\n");

        // Desligar todos os periféricos
        motor_stop(MOTOR_DIR1, MOTOR_DIR2, MOTOR_POT); // Desligar o motor
        
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
