#include <stdio.h>

#include "motor_simulacao.h"
#include "produto.h"
#include "fila.h"
#include "etapa.h"
#include "pilha.h"

void gerar_produtos(Simulacao *simulacao) {
   if (simulacao == NULL) {
      return;
   }

   int produtos_restantes =
      simulacao->produto.qtde_produzir -
      simulacao->produtos_criados;

   if (produtos_restantes <= 0) {
      return;
   }

   int quantidade_gerar = simulacao->produto.taxa_por_seg;

   if (quantidade_gerar > produtos_restantes) {
      quantidade_gerar = produtos_restantes;
   }

   for (int i = 0; i < quantidade_gerar; i++) {
      Produto *novo_produto = criar_produto(
         simulacao->proximo_id_produto,
         simulacao->tick_atual
      );

      if (novo_produto == NULL) {
         printf("\nErro durante a geracao de produtos.\n");
         printf("Nao foi possivel criar o produto de ID %d.\n",
            simulacao->proximo_id_produto);
         return;
      }

      enfileirar(simulacao->fila_entrada, novo_produto);

      simulacao->produtos_criados++;
      simulacao->proximo_id_produto++;
   }
}

void inserir_produtos_primeira_etapa(Simulacao *simulacao) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *primeira_etapa = simulacao->linha.inicio;

   if (primeira_etapa == NULL) {
      return;
   }

   while (!fila_vazia(simulacao->fila_entrada) &&
      primeira_etapa->ocupacao_atual < primeira_etapa->capacidade) {

      Produto *produto = desenfileirar(simulacao->fila_entrada);

      if (produto == NULL) {
         return;
      }

      produto->tick_entrada = simulacao->tick_atual;

      produto->tempo_espera =
         produto->tick_entrada - produto->tick_criacao;

      produto->etapa_atual = primeira_etapa;
      produto->atividade_atual = primeira_etapa->atividade;
      produto->tempo_atividade = 0;

      inserir_produto_etapa(primeira_etapa, produto);
   }
}

void processar_atividades(Simulacao *simulacao) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual = simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      Produto *produto_atual = etapa_atual->produtos;

      while (produto_atual != NULL) {
         if (produto_atual->atividade_atual != NULL) {
            produto_atual->tempo_atividade++;

            if (produto_atual->tempo_atividade >=
               produto_atual->atividade_atual->duracao) {

               empilhar_historico(produto_atual, produto_atual->atividade_atual->id);

               produto_atual->atividade_atual = produto_atual->atividade_atual->atividade_prox;

               produto_atual->tempo_atividade = 0;
            }
         }

         produto_atual = produto_atual->produto_prox;
      }

      etapa_atual = etapa_atual->etapa_prox;
   }
}

void inserir_produto_concluido(
   Simulacao *simulacao,
   Produto *produto
) {
   produto->produto_prox = NULL;

   if (simulacao->lista_concluidos == NULL) {
      simulacao->lista_concluidos = produto;
      return;
   }

   Produto *produto_atual = simulacao->lista_concluidos;

   while (produto_atual->produto_prox != NULL) {
      produto_atual = produto_atual->produto_prox;
   }

   produto_atual->produto_prox = produto;
}

void movimentar_produtos(Simulacao *simulacao) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual = simulacao->linha.fim;

   while (etapa_atual != NULL) {
      Produto *produto_atual = etapa_atual->produtos;

      while (produto_atual != NULL) {
         Produto *produto_prox = produto_atual->produto_prox;

         if (produto_atual->atividade_atual == NULL) {
            Etapa *proxima_etapa = etapa_atual->etapa_prox;

            if (proxima_etapa == NULL) {
               Produto *produto_removido =
                  remover_produto_etapa(
                     etapa_atual,
                     produto_atual
                  );

               if (produto_removido != NULL) {
                  produto_removido->tick_saida =
                     simulacao->tick_atual;

                  produto_removido->tempo_total =
                     produto_removido->tick_saida -
                     produto_removido->tick_criacao;

                  produto_removido->etapa_atual = NULL;

                  inserir_produto_concluido(
                     simulacao,
                     produto_removido
                  );

                  simulacao->produtos_concluidos++;
               }
            } else if (
               proxima_etapa->ocupacao_atual <
               proxima_etapa->capacidade
            ) {
               Produto *produto_removido =
                  remover_produto_etapa(
                     etapa_atual,
                     produto_atual
                  );

               if (produto_removido != NULL) {
                  produto_removido->etapa_atual =
                     proxima_etapa;

                  produto_removido->atividade_atual =
                     proxima_etapa->atividade;

                  produto_removido->tempo_atividade = 0;

                  inserir_produto_etapa(
                     proxima_etapa,
                     produto_removido
                  );
               }
            }
         }

         produto_atual = produto_prox;
      }

      etapa_atual = etapa_atual->etapa_ant;
   }
}
