#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MSG_KEY 5678 // Chave da fila de mensagens

// Estrutura para mensagens do painel
typedef struct {
    long msg_type; // Tipo da mensagem (1 para comandos)
    char command[100]; // Comando enviado pelo painel
} Message;

/**
 * @brief Mostra o menu de opções do painel de comando
 *
 * A função display_menu() imprime o menu de opções do painel de comando
 * na saida padrão do usuário. Ela lista todas as opções possíveis para o usuário
 * e aguarda a escolha da opção desejada.
 */
void display_menu() {
    printf("\n==== PAINEL DE COMANDOS ====\n");
    printf("1 - Ligar Seta Esquerda\n");
    printf("2 - Ligar Seta Direita\n");
    printf("3 - Ligar Farol Baixo\n");
    printf("4 - Ligar Farol Alto\n");
    printf("5 - Acionar Pedal do Acelerador\n");
    printf("6 - Acionar Pedal do Freio\n");
    printf("7 - Desligar Seta Esquerda\n");
    printf("8 - Desligar Seta Direita\n");
    printf("9 - Desligar Farol Baixo\n");
    printf("10 - Desligar Farol Alto\n");
    printf("11 - Desligar Farol\n");
    printf("0 - Sair\n");
    printf("Escolha uma opção: ");
}


/**
 * @brief Envia uma mensagem com um comando para a fila de mensagens do
 *        controller.
 *
 * A função send_message() envia uma mensagem com um comando para a fila de
 * mensagens do controller. Ela utiliza o IPC message queue com a chave
 * MSG_KEY e o tipo de mensagem definido em msg_type. Caso a mensagem seja
 * enviada com sucesso, imprime na saída padrão o comando enviado.
 *
 * @param msg_queue_id ID da fila de mensagens do controller.
 * @param msg Mensagem a ser enviada com o comando.
 */
void send_message(int msg_queue_id, Message msg) {
 if (msgsnd(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 0) < 0) {
    perror("Erro ao enviar comando para a fila de mensagens");
    exit(EXIT_FAILURE);
 } else {
     printf("Comando enviado: %s\n", msg.command);
    }   
}


/**
 * @brief Função principal do programa Painel de Comando.
 *
 * Este programa implementa um painel que envia comandos para um controlador
 * por meio de uma fila de mensagens IPC. O usuário interage com o sistema
 * por um menu textual, escolhendo ações como controle de setas, faróis e pedais.
 *
 * Principais funcionalidades:
 *  - Criação ou acesso à fila de mensagens identificada por `MSG_KEY`.
 *  - Monitoramento de mensagens recebidas:
 *    - Se o comando "Encerrar" for detectado, o programa finaliza.
 *  - Envio de comandos escolhidos pelo usuário para o controlador.
 *
 * Comportamento:
 *  - Comandos são enviados com o tipo de mensagem 1.
 *  - A opção 0 no menu envia o comando "Encerrar" ao controlador e encerra o programa.
 *  - Opções inválidas exibem uma mensagem de erro e reapresentam o menu.
 *
 * @return Retorna 0 ao encerrar o programa.
 */
int main() {
    Message msg;
    int msg_queue_id, option;

    // Criar ou acessar a fila de mensagens
    msg_queue_id = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msg_queue_id < 0) {
        perror("Erro ao criar/acessar a fila de mensagens");
        exit(EXIT_FAILURE);
    }

    while (1) {
        
        // Verificar mensagens na fila
        if (msgrcv(msg_queue_id, &msg, sizeof(msg) - sizeof(long), 2, IPC_NOWAIT) > 0) {
            if (strcmp(msg.command, "Encerrar") == 0) {
                printf("\nControlador solicitou encerramento. Encerrando Painel de Comando...\n");
                return 0;
            }
        }
        
        display_menu();

        if (scanf("%d", &option) != 1) {
            printf("Erro: entrada inválida. Por favor, insira um número.\n");
            while (getchar() != '\n'); // Limpar o buffer de entrada
            continue;
        }
        getchar(); // Limpar o buffer do teclado

        msg.msg_type = 1; // Tipo da mensagem do Painel

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
                send_message(msg_queue_id, msg);
                return 0;
                break;
            default:
                printf("Opção inválida. Tente novamente.\n");
                continue;
        }
        send_message(msg_queue_id, msg);
    }

    return 0;
}
