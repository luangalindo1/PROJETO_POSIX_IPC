#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

// Definições
#define MSG_KEY 5678 // Chave da fila de mensagens

// Estrutura para mensagens do painel
typedef struct {
    long msg_type;        // Tipo da mensagem (1 para comandos)
    char command[100];    // Comando enviado pelo painel
} Message;

// Variáveis globais para facilitar o uso no handler
static int msg_queue_id; 


/**
 * @brief Mostra o menu de opções do painel de comando
 *
 * A função display_menu() imprime o menu de opções do painel de comando
 * na saida padrão do usuário. Ela lista todas as opções possíveis para o usuário
 * e aguarda a escolha da opção desejada.
 * 
 * @note Foram adicionados alguns comandos com relação ao trabalho 1, como
 * ligar/desligar o farol (ambos) e ligar/desligar o pisca-alerta.
 */
void display_menu() {
    printf("\n==== PAINEL DE COMANDOS ====\n");
    printf("1  - Ligar Seta Esquerda\n");
    printf("2  - Ligar Seta Direita\n");
    printf("3  - Ligar Pisca-Alerta\n");
    printf("4  - Ligar Farol Baixo\n");
    printf("5  - Ligar Farol Alto\n");
    printf("6  - Acionar Pedal do Acelerador\n");
    printf("7  - Acionar Pedal do Freio\n");
    printf("8  - Desligar Seta Esquerda\n");
    printf("9  - Desligar Seta Direita\n");
    printf("10 - Desligar Pisca-Alerta\n");
    printf("11 - Desligar Farol Baixo\n");
    printf("12 - Desligar Farol Alto\n");
    printf("13 - Desligar Farol (ambos)\n");
    printf("0  - Sair\n");
    printf("Escolha uma opção: ");
}


/**
 * @brief Envia uma mensagem com um comando para a fila de mensagens do
 *        controller.
 *
 * A função send_message() envia uma mensagem com um comando para a fila de
 * mensagens do controller. Ela utiliza o IPC message queue com a chave
 * MSG_KEY e o tipo de mensagem definido em msg_type. Ela também imprime na
 * saída padrão o comando enviado.
 *
 * @note Com a msg_queue_id como variável global, 
 *       acabei removendo esse argumento.
 * 
 * @param msg Mensagem a ser enviada com o comando.
 */
void send_message(Message msg) {
    if (msgsnd(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
        perror("Erro ao enviar comando para a fila de mensagens");
        exit(EXIT_FAILURE);
    } else {
        printf("Comando enviado: %s\n", msg.command);
    }
}


/**
 * @brief Handler para o sinal SIGINT (Ctrl + C)
 *
 * Caso o sinal seja SIGINT, o handler imprime uma mensagem de encerramento,
 * envia o comando "Encerrar" para o controller e finaliza o programa
 * com o status 0.
 *
 * @param signum Sinal recebido pelo programa.
 */
void sigint_handler(int signal) {
    if (signal == SIGINT) {
        printf("\nEncerrando via Ctrl + C...\n");
        
        // Enviar o comando "Encerrar" para o controller
        Message msg;
        msg.msg_type = 1; 
        strcpy(msg.command, "Encerrar");
        send_message(msg);

        // Sair do programa
        exit(0);
    }
}


/**
 * @brief Limpa o buffer de entrada padrão (stdin) até encontrar um '\n' ou EOF.
 *
 * Essa função é útil para limpar o buffer de entrada
 * quando se lê um caractere com scanf e não se deseja
 * que o caractere seja lido novamente na próxima
 * interação com o usuário.
 */
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


/**
 * @brief Função principal do Painel de Comando.
 *
 * Instala um tratador para o sinal SIGINT (Ctrl + C) e
 * cria ou acessa a fila de mensagens com a chave MSG_KEY.
 *
 * Então, o programa entra em um loop principal que verifica
 * constantemente se o controlador enviou uma mensagem de
 * encerramento (tipo 2). Se sim, o programa sai com status 0.
 *
 * Caso contrário, o programa mostra o menu de opções para o
 * usuário e lê a entrada do usuário. Se a entrada for inválida,
 * o programa descarta a entrada e volta para o início do loop.
 *
 * Se a entrada for válida, o programa preenche a mensagem com o
 * tipo 1 (do Painel) e o comando escolhido pelo usuário, e
 * envia a mensagem para a fila de mensagens.
 *
 * O loop principal continua até que o usuário escolha a opção
 * 0 (Encerrar) ou o controlador envie uma mensagem de
 * encerramento.
 */
int main() {
    // Instalar o tratador de sinal SIGINT
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Erro ao instalar handler SIGINT");
        exit(EXIT_FAILURE);
    }

    // Criar ou acessar a fila de mensagens
    msg_queue_id = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msg_queue_id < 0) {
        perror("Erro ao criar/acessar a fila de mensagens");
        exit(EXIT_FAILURE);
    }

    Message msg;
    int option;
    int scan_result;

    // Loop principal
    while (1) {
        // Verificar se o controlador enviou uma mensagem de encerramento
        // (tipo 2)
        if (msgrcv(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 2, IPC_NOWAIT) > 0) {
            if (strcmp(msg.command, "Encerrar") == 0) {
                printf("\nControlador solicitou encerramento. Encerrando Painel de Comando...\n");
                return 0;
            }
        }

        // Mostrar o menu e ler opção do usuário
        display_menu();

        // Ler a entrada do usuário e verificar o retorno de scanf
        scan_result = scanf("%d", &option);
        if (scan_result != 1) {
            fprintf(stderr, "Entrada inválida. Por favor, insira um número.\n");
            clear_input_buffer(); // Limpar o buffer de entrada
            continue; // Voltar para o início do loop
        }

        clear_input_buffer(); // Descartar o '\n' do buffer

        // Preencher a mensagem com o tipo 1 (do Painel)
        msg.msg_type = 1;

        switch (option) {
        case 1:
            strcpy(msg.command, "Ligar Seta Esquerda");
            break;
        case 2:
            strcpy(msg.command, "Ligar Seta Direita");
            break;
        case 3:
            strcpy(msg.command, "Ligar Pisca-Alerta");
            break;
        case 4:
            strcpy(msg.command, "Ligar Farol Baixo");
            break;
        case 5:
            strcpy(msg.command, "Ligar Farol Alto");
            break;
        case 6:
            strcpy(msg.command, "Acionar Pedal do Acelerador");
            break;
        case 7:
            strcpy(msg.command, "Acionar Pedal do Freio");
            break;
        case 8:
            strcpy(msg.command, "Desligar Seta Esquerda");
            break;
        case 9:
            strcpy(msg.command, "Desligar Seta Direita");
            break;
        case 10:
            strcpy(msg.command, "Desligar Pisca-Alerta");
            break;
        case 11:
            strcpy(msg.command, "Desligar Farol Baixo");
            break;
        case 12:
            strcpy(msg.command, "Desligar Farol Alto");
            break;
        case 13:
            strcpy(msg.command, "Desligar Farol");
            break;
        case 0:
            printf("Encerrando Painel de Comandos...\n");
            printf("Encerrando controlador...\n\n");
            strcpy(msg.command, "Encerrar");
            send_message(msg);
            return 0;
        default:
            printf("Opção inválida. Tente novamente.\n");
            continue;
        }

        // Enviar a mensagem com o comando escolhido
        send_message(msg);
    }

    return 0;
}
