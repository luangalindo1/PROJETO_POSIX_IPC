/**
 * command_panel.c
 *
 * Versão adaptada para verificar o retorno de scanf e tratar entradas inválidas.
 */

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

// Função para exibir o menu de opções
void display_menu() {
    printf("\n==== PAINEL DE COMANDOS ====\n");
    printf("1  - Ligar Seta Esquerda\n");
    printf("2  - Ligar Seta Direita\n");
    printf("3  - Ligar Farol Baixo\n");
    printf("4  - Ligar Farol Alto\n");
    printf("5  - Acionar Pedal do Acelerador\n");
    printf("6  - Acionar Pedal do Freio\n");
    printf("7  - Desligar Seta Esquerda\n");
    printf("8  - Desligar Seta Direita\n");
    printf("9  - Desligar Farol Baixo\n");
    printf("10 - Desligar Farol Alto\n");
    printf("11 - Desligar Farol (ambos)\n");
    printf("0  - Sair\n");
    printf("Escolha uma opção: ");
}

// Função para enviar a mensagem para a fila
void send_message(Message msg) {
    if (msgsnd(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
        perror("Erro ao enviar comando para a fila de mensagens");
        exit(EXIT_FAILURE);
    } else {
        printf("Comando enviado: %s\n", msg.command);
    }
}

// Handler para SIGINT (Ctrl + C)
void sigint_handler(int signum) {
    if (signum == SIGINT) {
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

// Função para limpar o buffer de entrada
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Função principal
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
                strcpy(msg.command, "Ligar Farol Baixo");
                break;
            case 4:
                strcpy(msg.command, "Ligar Farol Alto");
                break;
            case 5:
                strcpy(msg.command, "Acionar Pedal do Acelerador");
                break;
            case 6:
                strcpy(msg.command, "Acionar Pedal do Freio");
                break;
            case 7:
                strcpy(msg.command, "Desligar Seta Esquerda");
                break;
            case 8:
                strcpy(msg.command, "Desligar Seta Direita");
                break;
            case 9:
                strcpy(msg.command, "Desligar Farol Baixo");
                break;
            case 10:
                strcpy(msg.command, "Desligar Farol Alto");
                break;
            case 11:
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
