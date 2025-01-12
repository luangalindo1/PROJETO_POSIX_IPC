// Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h> 
#include <signal.h>
#include <stdbool.h>

// Definições de pinos para os componentes

// Direção
#define MOTOR_DIR1 17 // Direção 1 (OUT)
#define MOTOR_DIR2 18 // Direção 2 (OUT)

// Motor
#define MOTOR_POT 23 // Potência do Motor (PWM) (OUT)

// Pedais
#define FREIO_INT 24 // Intensidade do Pedal de Freio (PWM) (OUT)
#define PEDAL_AC 27  // Pedal do Acelerador (IN)
#define PEDAL_FR 22  // Pedal do Freio (IN)

// Sensores Hall
#define SENSOR_HALL_MOTOR 11      // Sensor Hall do Motor (IN)
#define SENSOR_HALL_RODA_A 5      // Sensor Hall da Roda A (IN)
#define SENSOR_HALL_RODA_B 6      // Sensor Hall da Roda B (IN)

// Faróis e luzes
#define FAROL_BAIXO 19           // Luzes de Farol Baixo (OUT)
#define FAROL_ALTO 26            // Luzes de Farol Alto (OUT)
#define LUZ_FREIO 25             // Luzes de Freio (OUT)
#define LUZ_SETA_ESQ 8           // Luz da Seta Esquerda (OUT)
#define LUZ_SETA_DIR 7           // Luz da Seta Direita (OUT)
#define LUZ_TEMP_MOTOR 12        // Luz de Alerta da Temperatura do Motor (OUT)

// Comandos
#define COMANDO_FAROL 16         // Comando de Ligar/Desligar Farol (IN)
#define COMANDO_FAROL_ALTO 1     // Comando de Ligar/Desligar Farol Alto (IN)
#define COMANDO_SETA_ESQ 20      // Comando de Ligar/Desligar Seta Esquerda (IN)
#define COMANDO_SETA_DIR 21      // Comando de Ligar/Desligar Seta Direita (IN)

// Cruise Control
#define CC_RES 13                // Comando de Cruise Control (IN)
#define CC_CANCEL 0              // Comando de Cancelar Cruise Control (IN)

// OUTROS
#define WHEEL_DIAMETER 1.0        // Diâmetro das Rodas (m)