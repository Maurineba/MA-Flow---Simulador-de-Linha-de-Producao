#include <stdio.h>
#include <stdlib.h>

#include "motor_simulacao.h"
#include "produto.h"
#include "fila.h"
#include "etapa.h"
#include "pilha.h"
#include "arvore.h"
#include "relatorio.h"

#define TAXA_RETORNO_QUALIDADE 0.02f

float gerar_probabilidade() {
   return (float) rand() / RAND_MAX;
}

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

   int quantidade_gerar =
      simulacao->produto.taxa_por_seg;

   if (quantidade_gerar > produtos_restantes) {
      quantidade_gerar = produtos_restantes;
   }

   for (int i = 0; i < quantidade_gerar; i++) {
      Produto *novo_produto = criar_produto(
         simulacao->proximo_id_produto,
         simulacao->tick_atual
      );

      if (novo_produto == NULL) {
         printf(
            "\nErro durante a geracao de produtos.\n"
         );

         printf(
            "Nao foi possivel criar o produto de ID %d.\n",
            simulacao->proximo_id_produto
         );

         return;
      }

      enfileirar(
         simulacao->fila_entrada,
         novo_produto
      );

      simulacao->produtos_criados++;
      simulacao->proximo_id_produto++;
   }
}

void inserir_produtos_primeira_etapa(
   Simulacao *simulacao
) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *primeira_etapa =
      simulacao->linha.inicio;

   if (primeira_etapa == NULL) {
      return;
   }

   while (
      !fila_vazia(simulacao->fila_entrada) &&
      primeira_etapa->ocupacao_atual <
         primeira_etapa->capacidade
   ) {
      Produto *produto =
         desenfileirar(simulacao->fila_entrada);

      if (produto == NULL) {
         return;
      }

      produto->tick_entrada =
         simulacao->tick_atual;

      produto->tempo_espera =
         produto->tick_entrada -
         produto->tick_criacao;

      produto->etapa_atual =
         primeira_etapa;

      produto->atividade_atual =
         primeira_etapa->atividade;

      produto->tempo_atividade = 0;
      produto->aguardando_retorno = 0;

      inserir_produto_arvore(
         &simulacao->raiz_produtos_ativos,
         produto
      );

      inserir_produto_etapa(
         primeira_etapa,
         produto
      );
   }
}

int deve_retornar_etapa_anterior(
   const Produto *produto,
   const Etapa *etapa
) {
   if (
      produto == NULL ||
      etapa == NULL ||
      etapa->etapa_ant == NULL ||
      etapa->atividade == NULL
   ) {
      return 0;
   }

   if (
      produto->atividade_atual !=
      etapa->atividade
   ) {
      return 0;
   }

   return gerar_probabilidade() <
      TAXA_RETORNO_QUALIDADE;
}

void movimentar_retornos(
   Simulacao *simulacao
) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      Etapa *etapa_anterior =
         etapa_atual->etapa_ant;

      Produto *produto_atual =
         etapa_atual->produtos;

      while (produto_atual != NULL) {
         Produto *produto_prox =
            produto_atual->produto_prox;

         if (
            produto_atual->aguardando_retorno &&
            etapa_anterior != NULL &&
            etapa_anterior->ocupacao_atual <
               etapa_anterior->capacidade
         ) {
            Produto *produto_removido =
               remover_produto_etapa(
                  etapa_atual,
                  produto_atual
               );

            if (produto_removido != NULL) {

               remover_historico_etapa(
                  produto_removido,
                  etapa_anterior->id
               );

               produto_removido->etapa_atual =
                  etapa_anterior;

               produto_removido->atividade_atual =
                  etapa_anterior->atividade;

               produto_removido->tempo_atividade = 0;
               produto_removido->aguardando_retorno = 0;

               inserir_produto_etapa(
                  etapa_anterior,
                  produto_removido
               );
            }
         }

         produto_atual = produto_prox;
      }

      etapa_atual =
         etapa_atual->etapa_prox;
   }
}

void processar_atividades(
   Simulacao *simulacao
) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      Produto *produto_atual =
         etapa_atual->produtos;

      while (produto_atual != NULL) {
         Produto *produto_prox =
            produto_atual->produto_prox;

         if (
            produto_atual->atividade_atual != NULL &&
            !produto_atual->aguardando_retorno
         ) {
            produto_atual->tempo_atividade++;

            if (
               produto_atual->tempo_atividade >=
               produto_atual->atividade_atual->duracao
            ) {

               if (
                  deve_retornar_etapa_anterior(
                     produto_atual,
                     etapa_atual
                  )
               ) {
                  produto_atual->falhas++;
                  etapa_atual->quantidade_falhas++;

                  produto_atual
                     ->atividade_atual
                     ->quantidade_falhas++;

                  produto_atual->aguardando_retorno = 1;
                  produto_atual->atividade_atual = NULL;
                  produto_atual->tempo_atividade = 0;

                  produto_atual = produto_prox;
                  continue;
               }

               float sorteio_etapa =
                  gerar_probabilidade();

               float sorteio_atividade =
                  gerar_probabilidade();

               int ocorreu_falha =
                  sorteio_etapa <
                     etapa_atual->taxa_falha ||
                  sorteio_atividade <
                     produto_atual
                        ->atividade_atual
                        ->taxa_falha;

               if (!ocorreu_falha) {
                  empilhar_historico(
                     produto_atual,
                     etapa_atual->id,
                     produto_atual
                        ->atividade_atual
                        ->id
                  );

                  produto_atual->atividade_atual =
                     produto_atual
                        ->atividade_atual
                        ->atividade_prox;

                  produto_atual->tempo_atividade = 0;
               } else {
                  produto_atual->falhas++;
                  etapa_atual->quantidade_falhas++;

                  produto_atual
                     ->atividade_atual
                     ->quantidade_falhas++;

                  float tipo_falha =
                     gerar_probabilidade();

                  if (tipo_falha < 0.80f) {
                     remover_historico_etapa(
                        produto_atual,
                        etapa_atual->id
                     );

                     produto_atual->atividade_atual =
                        etapa_atual->atividade;

                     produto_atual->tempo_atividade = 0;
                  } else {
                     Produto *produto_descartado =
                        remover_produto_etapa(
                           etapa_atual,
                           produto_atual
                        );

                     if (produto_descartado != NULL) {
                        produto_descartado->tick_saida =
                           simulacao->tick_atual;

                        produto_descartado->tempo_total =
                           produto_descartado->tick_saida -
                           produto_descartado->tick_criacao;

                        produto_descartado->etapa_atual =
                           NULL;

                        produto_descartado->atividade_atual =
                           NULL;

                        produto_descartado->tempo_atividade =
                           0;

                        produto_descartado
                           ->aguardando_retorno = 0;

                        simulacao->raiz_produtos_ativos =
                           remover_produto_arvore(
                              simulacao
                                 ->raiz_produtos_ativos,
                              produto_descartado->id
                           );

                        empilhar_descarte(
                           &simulacao->descartados,
                           produto_descartado
                        );
                     }
                  }
               }
            }
         }

         produto_atual = produto_prox;
      }

      etapa_atual =
         etapa_atual->etapa_prox;
   }
}

void inserir_produto_concluido(
   Simulacao *simulacao,
   Produto *produto
) {
   if (
      simulacao == NULL ||
      produto == NULL
   ) {
      return;
   }

   produto->produto_prox = NULL;

   if (simulacao->lista_concluidos == NULL) {
      simulacao->lista_concluidos = produto;
      return;
   }

   Produto *produto_atual =
      simulacao->lista_concluidos;

   while (produto_atual->produto_prox != NULL) {
      produto_atual =
         produto_atual->produto_prox;
   }

   produto_atual->produto_prox = produto;
}

void movimentar_produtos(
   Simulacao *simulacao
) {
   if (simulacao == NULL) {
      return;
   }

   movimentar_retornos(simulacao);

   Etapa *etapa_atual =
      simulacao->linha.fim;

   while (etapa_atual != NULL) {
      Produto *produto_atual =
         etapa_atual->produtos;

      while (produto_atual != NULL) {
         Produto *produto_prox =
            produto_atual->produto_prox;

         if (
            produto_atual->atividade_atual == NULL &&
            !produto_atual->aguardando_retorno
         ) {
            Etapa *proxima_etapa =
               etapa_atual->etapa_prox;

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
                  produto_removido->atividade_atual = NULL;
                  produto_removido->tempo_atividade = 0;
                  produto_removido->aguardando_retorno = 0;

                  simulacao->raiz_produtos_ativos =
                     remover_produto_arvore(
                        simulacao->raiz_produtos_ativos,
                        produto_removido->id
                     );

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
                  produto_removido->aguardando_retorno = 0;

                  inserir_produto_etapa(
                     proxima_etapa,
                     produto_removido
                  );
               }
            }
         }

         produto_atual = produto_prox;
      }

      etapa_atual =
         etapa_atual->etapa_ant;
   }
}

int verificar_pausa(
   Simulacao *simulacao,
   const char *arquivo_relatorio_parcial
) {
   char comando[10];

   printf(
      "\nTick %d concluido.\n",
      simulacao->tick_atual
   );

   printf(
      "[ENTER] continuar | "
      "[p] relatorio parcial | "
      "[q] encerrar: "
   );

   if (
      fgets(
         comando,
         sizeof(comando),
         stdin
      ) == NULL
   ) {
      return 0;
   }

   if (
      comando[0] == 'p' ||
      comando[0] == 'P'
   ) {
      gerar_relatorio_parcial(
         simulacao,
         arquivo_relatorio_parcial
      );

      printf(
         "Pressione ENTER para continuar."
      );

      if (
         fgets(
            comando,
            sizeof(comando),
            stdin
         ) == NULL
      ) {
         return 0;
      }
   } else if (
      comando[0] == 'q' ||
      comando[0] == 'Q'
   ) {
      printf(
         "Simulacao encerrada pelo usuario "
         "no tick %d.\n",
         simulacao->tick_atual
      );

      return 1;
   }

   return 0;
}

void atualizar_metricas_gargalo(
   Simulacao *simulacao
) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   while (
      etapa_atual != NULL &&
      etapa_atual->etapa_prox != NULL
   ) {
      Etapa *proxima_etapa =
         etapa_atual->etapa_prox;

      Produto *produto_atual =
         etapa_atual->produtos;

      int etapa_bloqueada = 0;

      while (produto_atual != NULL) {
         if (
            produto_atual->atividade_atual == NULL &&
            !produto_atual->aguardando_retorno &&
            proxima_etapa->ocupacao_atual >=
               proxima_etapa->capacidade
         ) {
            etapa_bloqueada = 1;
            break;
         }

         produto_atual =
            produto_atual->produto_prox;
      }

      if (etapa_bloqueada) {
         etapa_atual->ticks_bloqueada++;
      }

      etapa_atual =
         etapa_atual->etapa_prox;
   }
}

void executar_simulacao(
   Simulacao *simulacao,
   const char *arquivo_relatorio_parcial
) {
   if (
      simulacao == NULL ||
      arquivo_relatorio_parcial == NULL
   ) {
      return;
   }

   srand(simulacao->semente);

   while (
      simulacao->produtos_concluidos +
         simulacao->descartados.quantidade <
            simulacao->produto.qtde_produzir &&
      simulacao->tick_atual <
         simulacao->tempo_limite
   ) {
      processar_atividades(simulacao);
      movimentar_produtos(simulacao);
      gerar_produtos(simulacao);
      inserir_produtos_primeira_etapa(simulacao);
      atualizar_metricas_gargalo(simulacao);

      simulacao->tick_atual++;

      if (
         simulacao->tick_atual % 5 == 0 &&
         simulacao->produtos_concluidos +
            simulacao->descartados.quantidade <
               simulacao->produto.qtde_produzir &&
         simulacao->tick_atual <
            simulacao->tempo_limite
      ) {
         int encerrar =
            verificar_pausa(
               simulacao,
               arquivo_relatorio_parcial
            );

         if (encerrar) {
            break;
         }
      }
   }
}
