### MAKE SEM EXECUÇÃO AUTOMÁTICA ###


###############################################################################
# Variáveis de compilação
###############################################################################
CC        = gcc
CFLAGS    = -Wall -Wextra -O2
LIBM      = -lm
# Precisamos de -pthread para threads e -lrt para semáforos POSIX
LTHREADS  = -pthread   
LRT       = -lrt

###############################################################################
# Alvos (executáveis)
###############################################################################
all: command_panel controller sensor_sim

# Painel de comando
command_panel: command_panel.c
	$(CC) $(CFLAGS) -o $@ $<
	@echo "[OK] Gerado executável: $@"

# Controlador
controller: controller.c
	$(CC) $(CFLAGS) -o $@ $< $(LTHREADS) $(LRT) $(LIBM)
	@echo "[OK] Gerado executável: $@"

# Simulação dos sensores
sensor_sim: sensor_sim.c
	$(CC) $(CFLAGS) -o $@ $< $(LTHREADS) $(LIBM)
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
