#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ui.h"
#include "cruise_control.h"
#include "signal_handler.h"
#include "motor_control.h"
#include "sensor.h"

int main() {
    const int motor_dir1_pin = 17;
    const int motor_dir2_pin = 18;
    const int motor_pwm_pin = 23;
    const int sensor_pin_a = 2;
    const int sensor_pin_b = 3;
    const float wheel_diameter = 0.6;

    int cruise_control_enabled = 0;  // Inicializar a flag de controle de cruzeiro desativada
    int set_speed = 0;               // Inicializar a velocidade configurada
    int headlights = 0;              // Inicializar estado dos faróis

    // Configurar sinais de interrupção para um encerramento seguro
    setup_signal_handler();

    // Configurar motor
    if (motor_setup(motor_dir1_pin, motor_dir2_pin, motor_pwm_pin) < 0) {
        fprintf(stderr, "Erro ao configurar o motor.\n");
        return 1;
    }

    // Inicializar UI (opcionalmente útil para exibir informações)
    float distance_traveled = 0.0;
    int rpm = 0;
    ui_display(0.0, rpm, distance_traveled, cruise_control_enabled, set_speed, headlights);

    // Loop principal
    while (1) {
        char command = ui_get_user_command();

        switch (command) {
            case 'A':
            case 'D':
            case 'C':
            case 'X':
            case 'H':
                ui_process_command(command, &cruise_control_enabled, &set_speed, &headlights);
                break;
            case 'Q':
                printf("Encerrando o programa.\n");
                motor_stop(motor_dir1_pin, motor_dir2_pin, motor_pwm_pin);
                exit(0);
            default:
                printf("Comando inválido!\n");
        }

        // Se o Cruise Control estiver ativado, iniciar o loop do Cruise Control
        if (cruise_control_enabled) {
            cruise_control_loop();
        }

        // Atualizar UI com informações relevantes (opcional)
        ui_display(sensor_read_car_speed(sensor_pin_a, sensor_pin_b, wheel_diameter), rpm, distance_traveled, cruise_control_enabled, set_speed, headlights);

        // Aguardar um curto período antes de verificar os comandos novamente (opcional)
        usleep(100000); // Aguarda 100ms
    }

    return 0;
}
