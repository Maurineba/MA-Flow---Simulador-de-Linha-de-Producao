#include <stdio.h>
#include <stdlib.h>
#include "produto.h"

Produto* criar_produto(int id) {
   Produto* produto = malloc(sizeof(Produto));
   if (produto == NULL) {
      return NULL;
   }

   produto->id = id;
   produto->falhas = 0;
   produto->tempo_total = 0;

   return produto;
}

void descartar_produto(Produto* produto) {
   free(produto);
}
