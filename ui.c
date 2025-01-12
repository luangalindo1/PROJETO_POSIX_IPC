#include <stdio.h>
#include "ui.h"
#include "sensor.h"
#include "motor_control.h"
#include "cruise_control.h"

/**
 * @brief Exibe o painel de controle com informações do carro.
 *
 * @param speed Velocidade atual do carro (km/h).
 * @param rpm Rotação atual do motor (RPM).
 * @param distance Distância percorrida (km).
 * @param cruise_enabled Indica se o cruise control está ativo.
 * @param set_speed Velocidade configurada no cruise control (km/h).
 * @param headlights Estado dos faróis (0 = desligado, 1 = ligado).
 */
void ui_display(float speed, int rpm, float distance, int cruise_enabled, int set_speed, int headlights) {
    printf("\n===============================\n");
    printf("  PAINEL DE CONTROLE DO CARRO  \n");
    printf("===============================\n");
    printf("Velocidade Atual: %.2f km/h\n", speed);
    printf("Rotação do Motor: %d RPM\n", rpm);
    printf("Distância Percorrida: %.2f km\n", distance);
    printf("Cruise Control: %s\n", cruise_enabled ? "Ativado" : "Desativado");
    if (cruise_enabled) {
        printf("Velocidade Configurada (Cruise Control): %d km/h\n", set_speed);
    }
    printf("Faróis: %s\n", headlights ? "Ligados" : "Desligados");
    printf("===============================\n");
}

/**
 * @brief Captura os comandos do usuário via terminal.
 *
 * @return Comando digitado pelo usuário.
 */
char ui_get_user_command() {
    char command;

    printf("\nCOMANDOS DISPONÍVEIS:\n");
    printf("A - Aumentar velocidade do Cruise Control em 5 km/h\n");
    printf("D - Diminuir velocidade do Cruise Control em 5 km/h\n");
    printf("C - Ativar Cruise Control\n");
    printf("X - Desativar Cruise Control\n");
    printf("H - Alternar estado dos faróis\n");
    printf("Q - Sair do programa\n");
    printf("Digite o comando: ");

    scanf(" %c", &command);
    return command;
}

/**
 * @brief Processa os comandos do usuário e atualiza o sistema.
 *
 * @param command Comando digitado pelo usuário.
 * @param cruise_enabled Ponteiro para o estado do cruise control.
 * @param set_speed Ponteiro para a velocidade configurada no cruise control.
 * @param headlights Ponteiro para o estado dos faróis.
 */
void ui_process_command(char command, int *cruise_enabled, int *set_speed, int *headlights) {
    switch (command) {
        case 'A':
            if (*cruise_enabled) {
                cruise_control_adjust_speed(1);
                (*set_speed)++;
                printf("Velocidade do Cruise Control aumentada para %d km/h\n", *set_speed);
            } else {
                printf("Cruise Control está desativado!\n");
            }
            break;
        case 'D':
            if (*cruise_enabled) {
                cruise_control_adjust_speed(-1);
                (*set_speed)--;
                if (*set_speed < 0) *set_speed = 0;
                printf("Velocidade do Cruise Control reduzida para %d km/h\n", *set_speed);
            } else {
                printf("Cruise Control está desativado!\n");
            }
            break;
        case 'C':
            *cruise_enabled = 1;
            cruise_control_enable(*set_speed);
            break;
        case 'X':
            *cruise_enabled = 0;
            cruise_control_disable();
            break;
        case 'H':
            *headlights = !*headlights;
            printf("Faróis %s\n", *headlights ? "ligados" : "desligados");
            break;
        case 'Q':
            printf("Encerrando o programa.\n");
            break;
        default:
            printf("Comando inválido!\n");
    }
}
