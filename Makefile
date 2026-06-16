CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c \
      src/simulacao.c \
      src/parser.c \
      src/etapa.c \
      src/atividade.c \
      src/produto.c \
      src/fila.c \
      src/motor_simulacao.c \
      src/pilha.c

TARGET = simulador

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) simulador.exe

rebuild: clean all
