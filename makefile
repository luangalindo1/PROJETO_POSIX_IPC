# Variáveis
CC = gcc                   # Compilador C
CFLAGS = -Wall -g #-M      # Opções do compilador: avisos e modo de depuração
LDFLAGS = -lwiringPi -lpthread -lrt # Bibliotecas necessárias
TARGET = car_control       # Nome do executável

# Busca todos os arquivos .c no diretório atual
SRC = $(wildcard *.c)
# Converte os arquivos .c para .o
OBJ = $(SRC:.c=.o)

# Regra principal
all: $(TARGET)

# Regra para gerar o executável
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regra para compilar os arquivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza do projeto
clean:
	rm -f $(TARGET) *.o

# Evita conflitos de nomes com arquivos reais
.PHONY: all clean
