#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pilha.h"

static void empilhar_pilha_historico(
   PilhaHistorico *pilha,
   int etapa_id,
   int atividade_id,
   const char *atividade_nome,
   int tentativa,
   int tick_fila,
   int tick_inicio,
   int tick_fim,
   int ticks_etapa,
   int sucesso
) {
   if (pilha == NULL) {
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
   strncpy(
      novo_no->atividade_nome,
      atividade_nome != NULL ? atividade_nome : "",
      sizeof(novo_no->atividade_nome) - 1
   );
   novo_no->atividade_nome[
      sizeof(novo_no->atividade_nome) - 1
   ] = '\0';
   novo_no->tentativa = tentativa;
   novo_no->tick_fila = tick_fila;
   novo_no->tick_inicio = tick_inicio;
   novo_no->tick_fim = tick_fim;
   novo_no->ticks_etapa = ticks_etapa;
   novo_no->sucesso = sucesso;

   novo_no->prox = pilha->topo;
   pilha->topo = novo_no;
   pilha->quantidade++;
}

void empilhar_historico(
   Produto *produto,
   int etapa_id,
   int atividade_id,
   const char *atividade_nome,
   int tentativa,
   int tick_fila,
   int tick_inicio,
   int tick_fim,
   int ticks_etapa,
   int sucesso
) {
   if (produto == NULL) {
      return;
   }

   empilhar_pilha_historico(
      &produto->historico,
      etapa_id,
      atividade_id,
      atividade_nome,
      tentativa,
      tick_fila,
      tick_inicio,
      tick_fim,
      ticks_etapa,
      sucesso
   );
}

void empilhar_auditoria(
   Produto *produto,
   int etapa_id,
   int atividade_id,
   const char *atividade_nome,
   int tentativa,
   int tick_fila,
   int tick_inicio,
   int tick_fim,
   int ticks_etapa,
   int sucesso
) {
   if (produto == NULL) {
      return;
   }

   empilhar_pilha_historico(
      &produto->auditoria,
      etapa_id,
      atividade_id,
      atividade_nome,
      tentativa,
      tick_fila,
      tick_inicio,
      tick_fim,
      ticks_etapa,
      sucesso
   );
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

static void liberar_pilha_historico(
   PilhaHistorico *pilha
) {
   if (pilha == NULL) {
      return;
   }

   HistoricoNo *no_atual = pilha->topo;

   while (no_atual != NULL) {
      HistoricoNo *no_prox = no_atual->prox;

      free(no_atual);

      no_atual = no_prox;
   }

   pilha->topo = NULL;
   pilha->quantidade = 0;
}

void liberar_historico(Produto *produto) {
   if (produto == NULL) {
      return;
   }

   liberar_pilha_historico(&produto->historico);
   liberar_pilha_historico(&produto->auditoria);

   produto->historico.topo = NULL;
   produto->historico.quantidade = 0;
   produto->auditoria.topo = NULL;
   produto->auditoria.quantidade = 0;
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
