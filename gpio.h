#ifndef GPIO_H
#define GPIO_H

#define GPIO_OUT 1
#define GPIO_IN 0

// Configuração de GPIO
int gpio_setup(int pin, int direction);

// Operações em GPIO
int gpio_write(int pin, int value);
int gpio_read(int pin);

// Interrupções em GPIO
int gpio_set_interrupt(int pin, int edge, void (*callback)(void));

#endif // GPIO_H
