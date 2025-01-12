#include "basic_def.h"
#include "ui.h"
#include "cruise_control.h"
#include "signal_handler.h"
#include "motor_control.h"
#include "sensor.h"
#include "interrupt.h"
#include "pwm.h"
#include "gpio.h"

static bool cruise_control_enabled = FALSE;
int set_speed = 0;
int delta = 5;
int headlights = 0;
float distance_traveled = 0.0;
float current_speed = 0.0;
int rpm = 0;

void update_ui() {
    // Atualiza a interface do usuário com os dados mais recentes
    ui_display(current_speed, rpm, distance_traveled, cruise_control_enabled, set_speed, headlights);
}

void process_user_command() {
    char command = ui_get_user_command();
    
    switch (command) {
        case 'A':  // Aumentar velocidade do Cruise Control
            cruise_control_enabled = (cruise_control_enabled == FALSE) ? TRUE : cruise_control_enabled;
            cruise_control_adjust_speed(delta); // Aumenta a velocidade em 5 km/h
            break;

        case 'D':  // Diminuir velocidade do Cruise Control
            cruise_control_enabled = (cruise_control_enabled == FALSE) ? TRUE : cruise_control_enabled;
            cruise_control_adjust_speed(-delta); // Diminui a velocidade em 5 km/h
            break;

        case 'C':  // Ativar Cruise Control
            cruise_control_enable(current_speed);
            break;

        case 'X':  // Desativar Cruise Control
            cruise_control_enabled = FALSE;
            cruise_control_disable();
            break;

        case 'H':  // Alternar estado dos faróis
            headlights = !headlights;  // Alterna entre 0 e 1 (desligado/ligado)
            headlights ? gpio_write(COMANDO_FAROL, HIGH) : gpio_write(COMANDO_FAROL, LOW);
            printf("Faróis %s.\n", headlights ? "ligados" : "desligados");
            break;

        case 'Q':  // Sair do programa
            printf("Encerrando o programa.\n");
            motor_stop(MOTOR_DIR1, MOTOR_DIR2, MOTOR_POT);
            exit(0);

        default:
            printf("Comando inválido!\n");
    }
}


void handle_cruise_control() {
    // Se o Cruise Control estiver ativado, manter a velocidade configurada
    if (cruise_control_enabled) {
        cruise_control_loop();
    }
}

void sensor_update() {
    // Atualiza as variáveis de sensores
    current_speed = sensor_read_car_speed(SENSOR_HALL_RODA_A, SENSOR_HALL_RODA_B, WHEEL_DIAMETER);
    distance_traveled += (current_speed / 3600.0) * 0.1; // Incremento da distância (km) em 100ms
    rpm = sensor_read_motor_rpm(SENSOR_HALL_MOTOR); // Obter RPM do sensor
}

void interrupt_handler() {
    // Lida com a interrupção: lê sensores e atualiza a UI
    sensor_update();
    handle_cruise_control();
    update_ui();
}

int main() {
    // Inicialização do WiringPi
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "Erro ao inicializar o WiringPi.\n");
        return 1;
    }

    // Configuração inicial
    setup_signal_handler(); // Configurar tratador de sinais para encerramento seguro

    // Configuração do motor
    if (motor_setup(MOTOR_DIR1, MOTOR_DIR2, MOTOR_POT) < 0) {
        fprintf(stderr, "Erro ao configurar o motor.\n");
        return 1;
    }

    // Configurar interrupção para o botão de comando do usuário
    if (interrupt_setup(PEDAL_AC, INT_EDGE_RISING, process_user_command) < 0) {
        fprintf(stderr, "Erro ao configurar interrupção para comandos do usuário.\n");
        return 1;
    }

    // Configurar interrupção para leitura de sensor
    if (interrupt_setup(SENSOR_HALL_MOTOR, INT_EDGE_RISING, interrupt_handler) < 0) {
        fprintf(stderr, "Erro ao configurar interrupção para sensor.\n");
        return 1;
    }

    // Exibir painel inicial
    update_ui();

    // Espera por interrupções
    while (1) {
        // O programa agora estará "dormindo" aguardando interrupções.
        // Não há necessidade de um loop contínuo aqui.
        delay(100); // Pequena pausa para dar tempo de processar interrupções
    }

    return 0;
}
