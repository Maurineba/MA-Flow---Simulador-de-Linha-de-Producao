#include <stdio.h>

#include "arvore.h"

void inserir_produto_arvore(
   Produto **raiz,
   Produto *produto
) {
   if (raiz == NULL || produto == NULL) {
      return;
   }

   if (*raiz == NULL) {
      produto->arvore_esq = NULL;
      produto->arvore_dir = NULL;

      *raiz = produto;
      return;
   }

   if (produto->id < (*raiz)->id) {
      inserir_produto_arvore(
         &(*raiz)->arvore_esq,
         produto
      );
   } else if (produto->id > (*raiz)->id) {
      inserir_produto_arvore(
         &(*raiz)->arvore_dir,
         produto
      );
   }
}

Produto *buscar_produto_arvore(
   Produto *raiz,
   int id
) {
   if (raiz == NULL) {
      return NULL;
   }

   if (id == raiz->id) {
      return raiz;
   }

   if (id < raiz->id) {
      return buscar_produto_arvore(
         raiz->arvore_esq,
         id
      );
   }

   return buscar_produto_arvore(
      raiz->arvore_dir,
      id
   );
}

Produto *remover_produto_arvore(
   Produto *raiz,
   int id
) {
   if (raiz == NULL) {
      return NULL;
   }

   if (id < raiz->id) {
      raiz->arvore_esq =
         remover_produto_arvore(
            raiz->arvore_esq,
            id
         );

      return raiz;
   }

   if (id > raiz->id) {
      raiz->arvore_dir =
         remover_produto_arvore(
            raiz->arvore_dir,
            id
         );

      return raiz;
   }

   if (raiz->arvore_esq == NULL) {
      Produto *filho_direito = raiz->arvore_dir;

      raiz->arvore_esq = NULL;
      raiz->arvore_dir = NULL;

      return filho_direito;
   }

   if (raiz->arvore_dir == NULL) {
      Produto *filho_esquerdo = raiz->arvore_esq;

      raiz->arvore_esq = NULL;
      raiz->arvore_dir = NULL;

      return filho_esquerdo;
   }

   Produto *sucessor = raiz->arvore_dir;

   while (sucessor->arvore_esq != NULL) {
      sucessor = sucessor->arvore_esq;
   }

   raiz->arvore_dir =
      remover_produto_arvore(
         raiz->arvore_dir,
         sucessor->id
      );

   sucessor->arvore_esq = raiz->arvore_esq;
   sucessor->arvore_dir = raiz->arvore_dir;

   raiz->arvore_esq = NULL;
   raiz->arvore_dir = NULL;

   return sucessor;
}

void imprimir_produtos_ativos(
   const Produto *raiz
) {
   if (raiz == NULL) {
      return;
   }

   imprimir_produtos_ativos(
      raiz->arvore_esq
   );

   printf("Produto ativo: ID %d\n", raiz->id);

   imprimir_produtos_ativos(
      raiz->arvore_dir
   );
}
