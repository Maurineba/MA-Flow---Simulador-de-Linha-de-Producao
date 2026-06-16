#include <stdio.h>
#include <stdlib.h>

#include "fila.h"

FilaProduto *criar_fila() {
   FilaProduto *fila = malloc(sizeof(FilaProduto));

   if (fila == NULL) {
      printf("\nErro ao criar fila de produtos.\n");
      printf("Nao foi possivel alocar memoria para a fila.\n");
      return NULL;
   }

   fila->inicio = NULL;
   fila->fim = NULL;
   fila->quantidade = 0;

   return fila;
}

void enfileirar(FilaProduto *fila, Produto *produto) {
   if (fila == NULL || produto == NULL) {
      return;
   }

   produto->produto_prox = NULL;

   if (fila->inicio == NULL) {
      fila->inicio = produto;
      fila->fim = produto;
   } else {
      fila->fim->produto_prox = produto;
      fila->fim = produto;
   }

   fila->quantidade++;
}

Produto *desenfileirar(FilaProduto *fila) {
   if (fila == NULL || fila->inicio == NULL) {
      return NULL;
   }

   Produto *produto_removido = fila->inicio;

   fila->inicio = produto_removido->produto_prox;

   if (fila->inicio == NULL) {
      fila->fim = NULL;
   }

   produto_removido->produto_prox = NULL;
   fila->quantidade--;

   return produto_removido;
}

int fila_vazia(const FilaProduto *fila) {
   if (fila == NULL) {
      return 1;
   }

   return fila->inicio == NULL;
}

void liberar_fila(FilaProduto *fila) {
   if (fila == NULL) {
      return;
   }

   Produto *produto_atual = fila->inicio;

   while (produto_atual != NULL) {
      Produto *produto_prox = produto_atual->produto_prox;

      liberar_produto(produto_atual);

      produto_atual = produto_prox;
   }

   free(fila);
}
