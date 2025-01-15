### MAKE SEM EXECUÇÃO AUTOMÁTICA ###


###############################################################################
# Variáveis de compilação
###############################################################################
CC       = gcc
CFLAGS   = -Wall -Wextra -O2
LDFLAGS  = -pthread -lrt   # Precisamos de -pthread para threads e -lrt para semáforos POSIX

# Somente o controller precisa WiringPi (GPIO, PWM).
WIRINGPI = -lwiringPi
LIBM     = -lm

###############################################################################
# Alvos (executáveis)
###############################################################################
all: command_panel controller sensor_sim

# Painel de comando
command_panel: command_panel.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "[OK] Gerado executável: $@"

# Controlador (usa WiringPi)
controller: controller.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(WIRINGPI) $(LIBM)
	@echo "[OK] Gerado executável: $@"

# Simulador de sensores
sensor_sim: sensor_sim.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBM)
	@echo "[OK] Gerado executável: $@"

###############################################################################
# Limpeza
###############################################################################
clean:
	rm -f command_panel controller sensor_sim
	@echo "[OK] Limpeza concluída."

###############################################################################
# Fim do Makefile
###############################################################################
