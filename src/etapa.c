#include <stdio.h>
#include <stdlib.h>

#include "etapa.h"

Etapa *criar_etapa_por_linha(const char *linha) {
   Etapa *nova_etapa = malloc(sizeof(Etapa));

   if (nova_etapa == NULL) {
      printf("Erro ao alocar memoria para a etapa.\n");
      return NULL;
   }

   int valores_lidos = sscanf(
      linha,
      "%*s %d %d %d %f %29s",
      &nova_etapa->id,
      &nova_etapa->qtde_atividades,
      &nova_etapa->capacidade,
      &nova_etapa->taxa_falha,
      nova_etapa->nome
   );

   if (valores_lidos != 5) {
      printf("Erro ao interpretar linha de etapa: %s", linha);
      free(nova_etapa);
      return NULL;
   }

   nova_etapa->ocupacao_atual = 0;
   nova_etapa->atividade = NULL;
   nova_etapa->produtos = NULL;

   nova_etapa->etapa_ant = NULL;
   nova_etapa->etapa_prox = NULL;

   return nova_etapa;
}

void inserir_etapa(LinhaProducao *linha, Etapa *etapa) {
   if (linha == NULL || etapa == NULL) {
      return;
   }

   if (linha->inicio == NULL) {
      linha->inicio = etapa;
      linha->fim = etapa;
      return;
   }

   etapa->etapa_ant = linha->fim;
   linha->fim->etapa_prox = etapa;
   linha->fim = etapa;
}

int existe_etapa_com_id(const LinhaProducao *linha, int id) {
   if (linha == NULL) {
      return 0;
   }

   Etapa *etapa_atual = linha->inicio;

   while (etapa_atual != NULL) {
      if (etapa_atual->id == id) {
         return 1;
      }

      etapa_atual = etapa_atual->etapa_prox;
   }

   return 0;
}


void inserir_produto_etapa(Etapa *etapa, Produto *produto) {
   if (etapa == NULL || produto == NULL) {
      return;
   }

   if (etapa->ocupacao_atual >= etapa->capacidade) {
      return;
   }

   produto->produto_prox = NULL;

   if (etapa->produtos == NULL) {
      etapa->produtos = produto;
      etapa->ocupacao_atual++;
      return;
   }

   Produto *produto_atual = etapa->produtos;

   while (produto_atual->produto_prox != NULL) {
      produto_atual = produto_atual->produto_prox;
   }

   produto_atual->produto_prox = produto;
   etapa->ocupacao_atual++;
}

Produto *remover_produto_etapa(Etapa *etapa, Produto *produto) {
   if (etapa == NULL || produto == NULL) {
      return NULL;
   }

   Produto *produto_atual = etapa->produtos;
   Produto *produto_anterior = NULL;

   while (produto_atual != NULL) {
      if (produto_atual == produto) {
         if (produto_anterior == NULL) {
            etapa->produtos = produto_atual->produto_prox;
         } else {
            produto_anterior->produto_prox =
               produto_atual->produto_prox;
         }

         produto_atual->produto_prox = NULL;
         etapa->ocupacao_atual--;

         return produto_atual;
      }

      produto_anterior = produto_atual;
      produto_atual = produto_atual->produto_prox;
   }

   return NULL;
}
