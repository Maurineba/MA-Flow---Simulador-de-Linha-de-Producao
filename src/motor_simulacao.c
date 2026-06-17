#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "motor_simulacao.h"
#include "produto.h"
#include "fila.h"
#include "etapa.h"
#include "pilha.h"
#include "arvore.h"
#include "relatorio.h"

volatile sig_atomic_t pausa_solicitada = 0;

void tratar_sigint(int sinal) {
   (void) sinal;
   pausa_solicitada = 1;
}

float gerar_probabilidade() {
   return (float) rand() / RAND_MAX;
}

void iniciar_tentativa_etapa(Produto *produto, Etapa *etapa, int tick_atual) {
   if (produto == NULL || etapa == NULL) {
      return;
   }

   produto->etapa_atual = etapa;
   produto->atividade_atual = etapa->atividade;
   produto->tempo_atividade = 0;
   produto->aguardando_retorno = 0;
   produto->tick_entrada_etapa = tick_atual;
   produto->tick_entrada_atividade = tick_atual;
   produto->tick_inicio_atividade = -1;

   int maior_tentativa = 0;
   HistoricoNo *historico_atual = produto->auditoria.topo;

   while (historico_atual != NULL) {
      if (historico_atual->etapa_id == etapa->id && historico_atual->tentativa > maior_tentativa) {
         maior_tentativa = historico_atual->tentativa;
      }

      historico_atual = historico_atual->prox;
   }

   produto->etapa_tentativa_id = etapa->id;
   produto->tentativa_etapa = maior_tentativa + 1;
   produto->controle_qualidade_verificado = 0;
   produto->etapa_metricas_registradas = 0;

   etapa->entradas_na_etapa++;
}

void preparar_inicio_atividade(Produto *produto, int tick_atual) {
   if (produto == NULL || produto->atividade_atual == NULL) {
      return;
   }

   if (produto->tick_inicio_atividade < 0) {
      produto->tick_inicio_atividade = tick_atual;
   }
}

void registrar_metricas_etapa(Etapa *etapa, Produto *produto, int tick_saida) {
   if (etapa == NULL || produto == NULL || produto->etapa_metricas_registradas) {
      return;
   }

   int tempo_etapa = tick_saida - produto->tick_entrada_etapa;

   int tempo_fila = produto->tick_inicio_atividade - produto->tick_entrada_etapa;

   if (tempo_etapa < 0) {
      tempo_etapa = 0;
   }

   if (tempo_fila < 0) {
      tempo_fila = 0;
   }

   etapa->produtos_processados++;
   etapa->soma_tempos += tempo_etapa;
   etapa->soma_tempos_fila += tempo_fila;

   if (etapa->produtos_processados == 1 || tempo_etapa < etapa->tempo_minimo) {
      etapa->tempo_minimo = tempo_etapa;
   }

   if (tempo_etapa > etapa->tempo_maximo) {
      etapa->tempo_maximo = tempo_etapa;
   }

   produto->etapa_metricas_registradas = 1;
}

void registrar_execucao_atividade(
   Simulacao *simulacao,
   Etapa *etapa,
   Produto *produto,
   int sucesso
) {
   if (simulacao == NULL || etapa == NULL || produto == NULL || produto->atividade_atual == NULL) {
      return;
   }

   Atividade *atividade = produto->atividade_atual;

   int tick_fim = simulacao->tick_atual + 1;

   int tick_inicio = produto->tick_inicio_atividade;

   if (tick_inicio < 0) {
      tick_inicio = simulacao->tick_atual;
   }

   int tick_fila = tick_inicio - produto->tick_entrada_atividade;

   int ticks_etapa = tick_fim - produto->tick_entrada_etapa;

   if (tick_fila < 0) {
      tick_fila = 0;
   }

   if (ticks_etapa < 0) {
      ticks_etapa = 0;
   }

   atividade->produtos_processados++;
   atividade->soma_tempos_fila += tick_fila;
   atividade->soma_tempos_totais += tick_fim - produto->tick_entrada_atividade;

   empilhar_auditoria(
      produto,
      etapa->id,
      atividade->id,
      atividade->nome,
      produto->tentativa_etapa,
      tick_fila,
      tick_inicio,
      tick_fim,
      ticks_etapa,
      sucesso
   );

   if (sucesso) {
      empilhar_historico(
         produto,
         etapa->id,
         atividade->id,
         atividade->nome,
         produto->tentativa_etapa,
         tick_fila,
         tick_inicio,
         tick_fim,
         ticks_etapa,
         sucesso
      );
   }
}

void preparar_proxima_atividade(Produto *produto, int tick_entrada_atividade) {
   if (produto == NULL) {
      return;
   }

   produto->tempo_atividade = 0;
   produto->tick_entrada_atividade = tick_entrada_atividade;
   produto->tick_inicio_atividade = -1;
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

         printf("Nao foi possivel criar o produto de ID %d.\n", simulacao->proximo_id_produto);

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

   while (
      !fila_vazia(simulacao->fila_entrada) &&
      primeira_etapa->ocupacao_atual <
      primeira_etapa->capacidade
   ) {
      Produto *produto = desenfileirar(simulacao->fila_entrada);

      if (produto == NULL) {
         return;
      }

      produto->tick_entrada = simulacao->tick_atual;

      produto->tempo_espera =
      produto->tick_entrada -
      produto->tick_criacao;

      produto->etapa_atual = primeira_etapa;

      iniciar_tentativa_etapa(produto, primeira_etapa, simulacao->tick_atual);

      inserir_produto_arvore(&simulacao->raiz_produtos_ativos, produto);

      inserir_produto_etapa(primeira_etapa, produto);
   }
}

int deve_retornar_etapa_anterior(const Produto *produto, const Etapa *etapa) {
   if (produto == NULL || etapa == NULL || etapa->etapa_ant == NULL || etapa->atividade == NULL) {
      return 0;
   }

   if (produto->atividade_atual != etapa->atividade) {
      return 0;
   }

   if (produto->controle_qualidade_verificado) {
      return 0;
   }

   return gerar_probabilidade() <
   etapa->taxa_falha;
}

void movimentar_retornos(Simulacao *simulacao) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual = simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      Etapa *etapa_anterior = etapa_atual->etapa_ant;

      Produto *produto_atual = etapa_atual->produtos;

      while (produto_atual != NULL) {
         Produto *produto_prox = produto_atual->produto_prox;

         if (
            produto_atual->aguardando_retorno &&
            etapa_anterior != NULL &&
            etapa_anterior->ocupacao_atual <
            etapa_anterior->capacidade
         ) {
            Produto *produto_removido = remover_produto_etapa(etapa_atual, produto_atual);

            if (produto_removido != NULL) {
               registrar_metricas_etapa(etapa_atual, produto_removido, simulacao->tick_atual);

               remover_historico_etapa(produto_removido, etapa_anterior->id);

               iniciar_tentativa_etapa(produto_removido, etapa_anterior, simulacao->tick_atual);

               inserir_produto_etapa(etapa_anterior, produto_removido);
            }
         }

         produto_atual = produto_prox;
      }

      etapa_atual = etapa_atual->etapa_prox;
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
         Produto *produto_prox = produto_atual->produto_prox;

         if (produto_atual->atividade_atual != NULL && !produto_atual->aguardando_retorno) {
            preparar_inicio_atividade(produto_atual, simulacao->tick_atual);

            if (
               !produto_atual->controle_qualidade_verificado &&
               produto_atual->atividade_atual == etapa_atual->atividade
            ) {
               if (deve_retornar_etapa_anterior(produto_atual, etapa_atual)) {
                  registrar_execucao_atividade(simulacao, etapa_atual, produto_atual, 0);

                  produto_atual->falhas++;
                  etapa_atual->quantidade_falhas++;

                  produto_atual->atividade_atual->quantidade_falhas++;

                  produto_atual->aguardando_retorno = 1;
                  produto_atual->atividade_atual = NULL;
                  produto_atual->tempo_atividade = 0;

                  produto_atual = produto_prox;
                  continue;
               }

               produto_atual->controle_qualidade_verificado = 1;
            }

            produto_atual->tempo_atividade++;

            if (produto_atual->tempo_atividade >= produto_atual->atividade_atual->duracao) {
               int ocorreu_falha =
                  gerar_probabilidade() < produto_atual->atividade_atual->taxa_falha;

               if (!ocorreu_falha) {
                  Atividade *atividade_finalizada = produto_atual->atividade_atual;

                  registrar_execucao_atividade(simulacao, etapa_atual, produto_atual, 1);

                  produto_atual->atividade_atual = atividade_finalizada->atividade_prox;

                  preparar_proxima_atividade(produto_atual, simulacao->tick_atual + 1);

                  if (produto_atual->atividade_atual == NULL) {
                     registrar_metricas_etapa(etapa_atual, produto_atual, simulacao->tick_atual + 1);
                  }
               } else {
                  registrar_execucao_atividade(simulacao, etapa_atual, produto_atual, 0);

                  produto_atual->falhas++;
                  etapa_atual->quantidade_falhas++;

                  produto_atual->atividade_atual->quantidade_falhas++;

                  float tipo_falha = gerar_probabilidade();

                  if (tipo_falha < 0.80f) {
                     registrar_metricas_etapa(etapa_atual, produto_atual, simulacao->tick_atual + 1);

                     remover_historico_etapa(produto_atual, etapa_atual->id);

                     iniciar_tentativa_etapa(produto_atual, etapa_atual, simulacao->tick_atual + 1);
                  } else {
                     registrar_metricas_etapa(etapa_atual, produto_atual, simulacao->tick_atual + 1);

                     Produto *produto_descartado = remover_produto_etapa(etapa_atual, produto_atual);

                     if (produto_descartado != NULL) {
                        produto_descartado->tick_saida = simulacao->tick_atual + 1;

                        produto_descartado->tempo_total =
                        produto_descartado->tick_saida -
                        produto_descartado->tick_criacao;

                        produto_descartado->etapa_atual = NULL;

                        produto_descartado->atividade_atual = NULL;

                        produto_descartado->tempo_atividade = 0;

                        produto_descartado->aguardando_retorno = 0;

                        simulacao->raiz_produtos_ativos =
                        remover_produto_arvore(simulacao->raiz_produtos_ativos, produto_descartado->id);

                        empilhar_descarte(&simulacao->descartados, produto_descartado);
                     }
                  }
               }
            }
         }

         produto_atual = produto_prox;
      }

      etapa_atual = etapa_atual->etapa_prox;
   }
}

void inserir_produto_concluido(Simulacao *simulacao, Produto *produto) {
   if (simulacao == NULL || produto == NULL) {
      return;
   }

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

   movimentar_retornos(simulacao);

   Etapa *etapa_atual = simulacao->linha.fim;

   while (etapa_atual != NULL) {
      Produto *produto_atual = etapa_atual->produtos;

      while (produto_atual != NULL) {
         Produto *produto_prox = produto_atual->produto_prox;

         if (produto_atual->atividade_atual == NULL && !produto_atual->aguardando_retorno) {
            Etapa *proxima_etapa = etapa_atual->etapa_prox;

            if (proxima_etapa == NULL) {
               Produto *produto_removido = remover_produto_etapa(etapa_atual, produto_atual);

               if (produto_removido != NULL) {
                  produto_removido->tick_saida = simulacao->tick_atual + 1;

                  produto_removido->tempo_total =
                  produto_removido->tick_saida -
                  produto_removido->tick_criacao;

                  produto_removido->etapa_atual = NULL;
                  produto_removido->atividade_atual = NULL;
                  produto_removido->tempo_atividade = 0;
                  produto_removido->aguardando_retorno = 0;

                  simulacao->raiz_produtos_ativos =
                  remover_produto_arvore(simulacao->raiz_produtos_ativos, produto_removido->id);

                  inserir_produto_concluido(simulacao, produto_removido);

                  simulacao->produtos_concluidos++;
               }
            } else if (
               proxima_etapa->ocupacao_atual <
               proxima_etapa->capacidade
            ) {
               Produto *produto_removido = remover_produto_etapa(etapa_atual, produto_atual);

               if (produto_removido != NULL) {
                  iniciar_tentativa_etapa(produto_removido, proxima_etapa, simulacao->tick_atual + 1);

                  inserir_produto_etapa(proxima_etapa, produto_removido);
               }
            }
         }

         produto_atual = produto_prox;
      }

      etapa_atual = etapa_atual->etapa_ant;
   }
}

int verificar_pausa(Simulacao *simulacao, const char *arquivo_relatorio_parcial) {
   char comando[10];

   printf("\nTick %d concluido.\n", simulacao->tick_atual);

   printf("[ENTER] continuar | [p] relatorio parcial | [q] encerrar: ");

   if (fgets(comando, sizeof(comando), stdin) == NULL) {
      return 0;
   }

   if (comando[0] == 'p' || comando[0] == 'P') {
      gerar_relatorio_parcial(simulacao, arquivo_relatorio_parcial);

      printf("Pressione ENTER para continuar.");

      if (fgets(comando, sizeof(comando), stdin) == NULL) {
         return 0;
      }
   } else if (
      comando[0] == 'q' ||
      comando[0] == 'Q'
   ) {
      printf("Simulacao encerrada pelo usuario no tick %d.\\n", simulacao->tick_atual);

      return 1;
   }

   return 0;
}

void atualizar_metricas_gargalo(Simulacao *simulacao) {
   if (simulacao == NULL) {
      return;
   }

   Etapa *etapa_atual = simulacao->linha.inicio;

   while (etapa_atual != NULL && etapa_atual->etapa_prox != NULL) {
      Etapa *proxima_etapa = etapa_atual->etapa_prox;

      Produto *produto_atual = etapa_atual->produtos;

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

         produto_atual = produto_atual->produto_prox;
      }

      if (etapa_bloqueada) {
         etapa_atual->ticks_bloqueada++;
      }

      etapa_atual = etapa_atual->etapa_prox;
   }
}

void executar_simulacao(Simulacao *simulacao, const char *arquivo_relatorio_parcial) {
   if (simulacao == NULL || arquivo_relatorio_parcial == NULL) {
      return;
   }

   srand(simulacao->semente);
   signal(SIGINT, tratar_sigint);

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

      if (pausa_solicitada) {
         pausa_solicitada = 0;

         int encerrar = verificar_pausa(simulacao, arquivo_relatorio_parcial);

         if (encerrar) {
            break;
         }
      }

      if (
         simulacao->tick_atual % 5 == 0 &&
         simulacao->produtos_concluidos +
         simulacao->descartados.quantidade <
         simulacao->produto.qtde_produzir &&
         simulacao->tick_atual <
         simulacao->tempo_limite
      ) {
         int encerrar = verificar_pausa(simulacao, arquivo_relatorio_parcial);

         if (encerrar) {
            break;
         }
      }
   }
}


