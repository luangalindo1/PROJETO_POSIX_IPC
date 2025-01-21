### MAKE SEM EXECUÇÃO AUTOMÁTICA ###


###############################################################################
# Variáveis de compilação
###############################################################################
CC       = gcc
CFLAGS   = -Wall -Wextra -O2
LDFLAGS  = -pthread -lrt   # Precisamos de -pthread para threads e -lrt para semáforos POSIX

# Somente o controlador precisa de WiringPi (GPIO, PWM).
WIRINGPI = -lwiringPi
LIBM     = -lm

###############################################################################
# Alvos (executáveis)
###############################################################################
all: command_panel controller

# Painel de comando
command_panel: command_panel.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "[OK] Gerado executável: $@"

# Controlador (usa WiringPi)
controller: controller.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(WIRINGPI) $(LIBM)
	@echo "[OK] Gerado executável: $@"

###############################################################################
# Limpeza
###############################################################################
clean:
	rm -f command_panel controller
	@echo "[OK] Limpeza concluída."

###############################################################################
# Fim do Makefile
###############################################################################
