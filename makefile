###############################################################################
# Makefile para Compilação e Execução Automática em Ambiente Headless
###############################################################################

# Variáveis de Compilação
CC        = gcc
CFLAGS    = -Wall -Wextra -O2
LDFLAGS   = -pthread -lrt
WIRINGPI  = -lwiringPi
LIBM      = -lm

# Executáveis
EXEC_COMMAND_PANEL = command_panel
EXEC_CONTROLLER    = controller
EXEC_SENSOR_SIM    = sensor_sim

# Alvo Padrão
all: $(EXEC_COMMAND_PANEL) $(EXEC_CONTROLLER) $(EXEC_SENSOR_SIM)

###############################################################################
# Regras de Compilação
###############################################################################

# Compilar command_panel
$(EXEC_COMMAND_PANEL): command_panel.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "[OK] Executável gerado: $@"

# Compilar controller (usa WiringPi e libm)
$(EXEC_CONTROLLER): controller.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(WIRINGPI) $(LIBM)
	@echo "[OK] Executável gerado: $@"

# Compilar sensor_sim (inclui libm)
#$(EXEC_SENSOR_SIM): sensor_sim.c
#	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBM)
#	@echo "[OK] Executável gerado: $@"

###############################################################################
# Regra para Executar os Executáveis em Background com Logs
###############################################################################

run: all
	@echo "Executando executáveis em background..."
	@./$(EXEC_COMMAND_PANEL) > command_panel.log 2>&1 &
	@./$(EXEC_CONTROLLER) > controller.log 2>&1 &
	@./$(EXEC_SENSOR_SIM) > sensor_sim.log 2>&1 &
	@echo "Executáveis rodando em background. Logs: command_panel.log, controller.log, sensor_sim.log"

###############################################################################
# Regra de Limpeza
###############################################################################

clean:
	rm -f $(EXEC_COMMAND_PANEL) $(EXEC_CONTROLLER) $(EXEC_SENSOR_SIM) command_panel.log controller.log sensor_sim.log
	@echo "[OK] Limpeza concluída."

###############################################################################
# Declaração de Alvos Falsos
###############################################################################

.PHONY: all run clean
