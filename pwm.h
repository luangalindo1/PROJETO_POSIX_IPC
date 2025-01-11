#ifndef PWM_H
#define PWM_H

// Configuração de PWM
int pwm_setup(int pin, int initialValue);

// Operações em PWM
int pwm_write(int pin, int dutyCycle);
int pwm_stop(int pin);

#endif // PWM_H
