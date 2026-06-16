#include <stdio.h>
#include <stdlib.h>

#include "atividade.h"

Atividade *criar_atividade_por_linha(const char *linha) {
   Atividade *nova_atividade = malloc(sizeof(Atividade));

   if (nova_atividade == NULL) {
      printf("Erro ao alocar memoria para a atividade.\n");
      return NULL;
   }

   int valores_lidos = sscanf(
      linha,
      "%*s %d %d %f %29s",
      &nova_atividade->id,
      &nova_atividade->duracao,
      &nova_atividade->taxa_falha,
      nova_atividade->nome
   );

   if (valores_lidos != 4) {
      printf("Erro ao interpretar linha de atividade: %s", linha);
      free(nova_atividade);
      return NULL;
   }

   nova_atividade->quantidade_falhas = 0;
   nova_atividade->atividade_prox = NULL;

   return nova_atividade;
}

void inserir_atividade(Etapa *etapa, Atividade *atividade) {
   if (etapa == NULL || atividade == NULL) {
      return;
   }

   if (etapa->atividade == NULL) {
      etapa->atividade = atividade;
      return;
   }

   Atividade *atividade_atual = etapa->atividade;

   while (atividade_atual->atividade_prox != NULL) {
      atividade_atual = atividade_atual->atividade_prox;
   }

   atividade_atual->atividade_prox = atividade;
}

int existe_atividade_com_id(const Etapa *etapa, int id) {
   if (etapa == NULL) {
      return 0;
   }

   Atividade *atividade_atual = etapa->atividade;

   while (atividade_atual != NULL) {
      if (atividade_atual->id == id) {
         return 1;
      }

      atividade_atual = atividade_atual->atividade_prox;
   }

   return 0;
}
