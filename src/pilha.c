#include <stdio.h>
#include <stdlib.h>

#include "pilha.h"

void empilhar_historico(
   Produto *produto,
   int etapa_id,
   int atividade_id
) {
   if (produto == NULL) {
      return;
   }

   HistoricoNo *novo_no =
      malloc(sizeof(HistoricoNo));

   if (novo_no == NULL) {
      printf(
         "Erro ao alocar memoria para o historico.\n"
      );

      return;
   }

   novo_no->etapa_id = etapa_id;
   novo_no->atividade_id = atividade_id;

   novo_no->prox = produto->historico.topo;
   produto->historico.topo = novo_no;
   produto->historico.quantidade++;
}

int desempilhar_historico(Produto *produto) {
   if (produto == NULL || produto->historico.topo == NULL) {
      return -1;
   }

   HistoricoNo *no_removido = produto->historico.topo;
   int atividade_id = no_removido->atividade_id;

   produto->historico.topo = no_removido->prox;
   produto->historico.quantidade--;

   free(no_removido);

   return atividade_id;
}

void remover_historico_etapa(
   Produto *produto,
   int etapa_id
) {
   if (produto == NULL) {
      return;
   }

   while (
      produto->historico.topo != NULL &&
      produto->historico.topo->etapa_id == etapa_id
   ) {
      desempilhar_historico(produto);
   }
}

void liberar_historico(Produto *produto) {
   if (produto == NULL) {
      return;
   }

   HistoricoNo *no_atual = produto->historico.topo;

   while (no_atual != NULL) {
      HistoricoNo *no_prox = no_atual->prox;

      free(no_atual);

      no_atual = no_prox;
   }

   produto->historico.topo = NULL;
   produto->historico.quantidade = 0;
}

void empilhar_descarte(
   PilhaDescarte *pilha,
   Produto *produto
) {
   if (pilha == NULL || produto == NULL) {
      return;
   }

   produto->produto_prox = pilha->topo;
   pilha->topo = produto;
   pilha->quantidade++;
}

Produto *desempilhar_descarte(PilhaDescarte *pilha) {
   if (pilha == NULL || pilha->topo == NULL) {
      return NULL;
   }

   Produto *produto_removido = pilha->topo;

   pilha->topo = produto_removido->produto_prox;
   produto_removido->produto_prox = NULL;
   pilha->quantidade--;

   return produto_removido;
}

void liberar_pilha_descarte(PilhaDescarte *pilha) {
   if (pilha == NULL) {
      return;
   }

   Produto *produto_atual = pilha->topo;

   while (produto_atual != NULL) {
      Produto *produto_prox = produto_atual->produto_prox;

      liberar_produto(produto_atual);

      produto_atual = produto_prox;
   }

   pilha->topo = NULL;
   pilha->quantidade = 0;
}
