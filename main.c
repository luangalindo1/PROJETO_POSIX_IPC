#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>  // Adicionando a biblioteca WiringPi
#include "ui.h"
#include "cruise_control.h"
#include "signal_handler.h"
#include "motor_control.h"
#include "sensor.h"

int main() {
    // Configuração de pinos
    const int motor_dir1_pin = 17;
    const int motor_dir2_pin = 18;
    const int motor_pwm_pin = 23;
    const int sensor_pin_a = 2;  // Sensor de velocidade
    const int sensor_pin_b = 3;  // Sensor de rotação
    const float wheel_diameter = 0.6; // Diâmetro da roda em metros

    // Variáveis do sistema
    int cruise_control_enabled = 0;  // Estado inicial do cruise control
    int set_speed = 0;               // Velocidade configurada no cruise control
    int headlights = 0;              // Estado dos faróis (0: desligados, 1: ligados)
    float distance_traveled = 0.0;   // Distância percorrida em km
    float current_speed = 0.0;       // Velocidade atual do veículo (km/h)
    int rpm = 0;                     // Rotação por minuto do motor

    // Inicialização do WiringPi
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "Erro ao inicializar o WiringPi.\n");
        return 1;
    }

    // Configuração inicial
    setup_signal_handler(); // Configurar tratador de sinais para encerramento seguro

    // Configuração do motor
    if (motor_setup(motor_dir1_pin, motor_dir2_pin, motor_pwm_pin) < 0) {
        fprintf(stderr, "Erro ao configurar o motor.\n");
        return 1;
    }

    // Exibir painel inicial
    ui_display(current_speed, rpm, distance_traveled, cruise_control_enabled, set_speed, headlights);

    // Loop principal
    while (1) {
        // Obter comando do usuário
        char command = ui_get_user_command();

        // Processar o comando
        switch (command) {
            case 'A':  // Aumentar velocidade do Cruise Control
            case 'D':  // Diminuir velocidade do Cruise Control
            case 'C':  // Ativar Cruise Control
            case 'X':  // Desativar Cruise Control
            case 'H':  // Alternar estado dos faróis
                ui_process_command(command, &cruise_control_enabled, &set_speed, &headlights);
                break;

            case 'Q':  // Sair do programa
                printf("Encerrando o programa.\n");
                motor_stop(motor_dir1_pin, motor_dir2_pin, motor_pwm_pin);
                exit(0);

            default:
                printf("Comando inválido!\n");
        }

        // Atualizar informações do veículo
        current_speed = sensor_read_car_speed(sensor_pin_a, sensor_pin_b, wheel_diameter);
        distance_traveled += (current_speed / 3600.0) * 0.1; // Incremento da distância (km) em 100ms
        rpm = sensor_read_motor_rpm(sensor_pin_a, sensor_pin_b); // Obter RPM do sensor

        // Se o Cruise Control estiver ativado, manter a velocidade configurada
        if (cruise_control_enabled) {
            cruise_control_loop();
        }

        // Atualizar UI com informações relevantes
        ui_display(current_speed, rpm, distance_traveled, cruise_control_enabled, set_speed, headlights);

        // Aguardar um curto intervalo antes de repetir o loop
        usleep(100000); // 100ms
    }

    return 0;
}
