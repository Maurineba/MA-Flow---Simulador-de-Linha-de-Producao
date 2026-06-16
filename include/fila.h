#ifndef FILA_H
#define FILA_H

#include "produto.h"

typedef struct FilaProduto {
   Produto *inicio;
   Produto *fim;
   int quantidade;

} FilaProduto;

FilaProduto *criar_fila();
void enfileirar(FilaProduto *fila, Produto *produto);
Produto *desenfileirar(FilaProduto *fila);
int fila_vazia(const FilaProduto *fila);
void liberar_fila(FilaProduto *fila);

#endif
