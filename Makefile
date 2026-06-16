CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

TARGET = simulador

SRC = src/main.c \
      src/simulacao.c \
      src/parser.c \
      src/etapa.c \
      src/atividade.c \
      src/produto.c \
      src/fila.c \
      src/motor_simulacao.c \
      src/pilha.c \
      src/arvore.c \
      src/relatorio.c

INPUT ?= input/test.txt

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET) $(INPUT)

clean:
	rm -f $(TARGET) $(TARGET).exe

