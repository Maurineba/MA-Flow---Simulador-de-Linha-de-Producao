#include <stdio.h>
#include <stdlib.h>

#include "simulacao.h"
#include "produto.h"
#include "pilha.h"

Simulacao *criar_simulacao() {
   Simulacao *simulacao = malloc(sizeof(Simulacao));

   if (simulacao == NULL) {
      printf("Erro ao alocar memoria para a simulacao.\n");
      return NULL;
   }

   simulacao->nome[0] = '\0';
   simulacao->semente = 0;
   simulacao->tempo_limite = 0;

   simulacao->produto.qtde_produzir = 0;
   simulacao->produto.taxa_por_seg = 0;
   simulacao->produto.nome[0] = '\0';

   simulacao->linha.qtde_etapas = 0;
   simulacao->linha.inicio = NULL;
   simulacao->linha.fim = NULL;

   simulacao->tick_atual = 0;
   simulacao->produtos_criados = 0;
   simulacao->proximo_id_produto = 1;

   simulacao->produtos_concluidos = 0;
   simulacao->lista_concluidos = NULL;

   simulacao->descartados.topo = NULL;
   simulacao->descartados.quantidade = 0;
   simulacao->raiz_produtos_ativos = NULL;

   simulacao->fila_entrada = criar_fila();
   if (simulacao->fila_entrada == NULL) {
      free(simulacao);
      return NULL;
   }

   return simulacao;
}

void liberar_simulacao(Simulacao *simulacao) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual = simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      Atividade *atividade_atual = etapa_atual->atividade;

      while (atividade_atual != NULL) {
         Atividade *atividade_prox = atividade_atual->atividade_prox;

         free(atividade_atual);

         atividade_atual = atividade_prox;
      }

      Produto *produto_atual = etapa_atual->produtos;

      while (produto_atual != NULL) {
         Produto *produto_prox = produto_atual->produto_prox;

         liberar_produto(produto_atual);

         produto_atual = produto_prox;
      }

      Etapa *etapa_prox = etapa_atual->etapa_prox;

      free(etapa_atual);

      etapa_atual = etapa_prox;
   }

   Produto *produto_concluido = simulacao->lista_concluidos;

   while (produto_concluido != NULL) {
      Produto *produto_prox = produto_concluido->produto_prox;

      liberar_produto(produto_concluido);

      produto_concluido = produto_prox;
   }
   
   liberar_pilha_descarte(&simulacao->descartados);
   liberar_fila(simulacao->fila_entrada);
   free(simulacao);
}
