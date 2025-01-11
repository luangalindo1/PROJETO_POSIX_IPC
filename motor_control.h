#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

// Constantes de direção do motor
#define MOTOR_FORWARD 1
#define MOTOR_REVERSE 2
#define MOTOR_BRAKE 3
#define MOTOR_FREE 4

// Configuração do motor
int motor_setup(int dir1, int dir2, int pwm);

// Operações no motor
void motor_set_direction(int dir1, int dir2, int direction);
void motor_set_speed(int pwm, int speed);
void motor_stop(int dir1, int dir2, int pwm);

#endif // MOTOR_CONTROL_H
