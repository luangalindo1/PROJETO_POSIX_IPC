#ifndef SENSOR_H
#define SENSOR_H



// Funções para leitura de sensores
int sensor_read_motor_rpm(int pin);
float sensor_read_car_speed(int pin_a, int pin_b, float wheel_diameter);
float sensor_calculate_distance(int revolutions, float wheel_diameter);

#endif // SENSOR_H
