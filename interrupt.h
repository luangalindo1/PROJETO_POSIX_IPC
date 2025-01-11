#ifndef INTERRUPT_H
#define INTERRUPT_H

// Configuração de Interrupções
int interrupt_setup(int pin, int edge, void (*callback)(void));
int interrupt_disable(int pin);

#endif // INTERRUPT_H
