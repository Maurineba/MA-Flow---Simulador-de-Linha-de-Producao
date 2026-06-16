#include <stdio.h>

#include "relatorio.h"

int calcular_falhas_totais(
   const Simulacao *simulacao
) {
   int falhas_totais = 0;

   Produto *produto_atual =
      simulacao->lista_concluidos;

   while (produto_atual != NULL) {
      falhas_totais += produto_atual->falhas;

      produto_atual =
         produto_atual->produto_prox;
   }

   produto_atual =
      simulacao->descartados.topo;

   while (produto_atual != NULL) {
      falhas_totais += produto_atual->falhas;

      produto_atual =
         produto_atual->produto_prox;
   }

   produto_atual =
      simulacao->fila_entrada->inicio;

   while (produto_atual != NULL) {
      falhas_totais += produto_atual->falhas;

      produto_atual =
         produto_atual->produto_prox;
   }

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      produto_atual =
         etapa_atual->produtos;

      while (produto_atual != NULL) {
         falhas_totais += produto_atual->falhas;

         produto_atual =
            produto_atual->produto_prox;
      }

      etapa_atual =
         etapa_atual->etapa_prox;
   }

   return falhas_totais;
}

float calcular_tempo_medio_total(
   const Simulacao *simulacao
) {
   if (simulacao->produtos_concluidos == 0) {
      return 0.0f;
   }

   int soma_tempos = 0;

   Produto *produto_atual =
      simulacao->lista_concluidos;

   while (produto_atual != NULL) {
      soma_tempos += produto_atual->tempo_total;

      produto_atual =
         produto_atual->produto_prox;
   }

   return (float) soma_tempos /
      simulacao->produtos_concluidos;
}

float calcular_tempo_medio_espera(
   const Simulacao *simulacao
) {
   if (simulacao->produtos_concluidos == 0) {
      return 0.0f;
   }

   int soma_espera = 0;

   Produto *produto_atual =
      simulacao->lista_concluidos;

   while (produto_atual != NULL) {
      soma_espera += produto_atual->tempo_espera;

      produto_atual =
         produto_atual->produto_prox;
   }

   return (float) soma_espera /
      simulacao->produtos_concluidos;
}

void escrever_historico_ordem(
   FILE *arquivo,
   const HistoricoNo *no,
   int *primeiro
) {
   if (no == NULL) {
      return;
   }

   escrever_historico_ordem(
      arquivo,
      no->prox,
      primeiro
   );

   if (!(*primeiro)) {
      fprintf(arquivo, " -> ");
   }

   fprintf(
      arquivo,
      "etapa_%d/atividade_%d",
      no->etapa_id,
      no->atividade_id
   );

   *primeiro = 0;
}

void escrever_historico(
   FILE *arquivo,
   const Produto *produto
) {
   fprintf(arquivo, "Trajetoria: ");

   if (produto->historico.topo == NULL) {
      fprintf(
         arquivo,
         "nenhuma atividade concluida\n"
      );

      return;
   }

   int primeiro = 1;

   escrever_historico_ordem(
      arquivo,
      produto->historico.topo,
      &primeiro
   );

   fprintf(arquivo, "\n");
}

void escrever_produto(
   FILE *arquivo,
   const Produto *produto,
   const char *modelo,
   const char *situacao
) {
   fprintf(
      arquivo,
      "\n--- PRODUTO %d ---\n",
      produto->id
   );

   fprintf(
      arquivo,
      "Modelo: %s\n",
      modelo
   );

   fprintf(
      arquivo,
      "Situacao: %s\n",
      situacao
   );

   fprintf(
      arquivo,
      "Criacao: tick %d\n",
      produto->tick_criacao
   );

   fprintf(
      arquivo,
      "Entrada na linha: tick %d\n",
      produto->tick_entrada
   );

   fprintf(
      arquivo,
      "Saida da linha: tick %d\n",
      produto->tick_saida
   );

   fprintf(
      arquivo,
      "Tempo total: %d ticks\n",
      produto->tempo_total
   );

   fprintf(
      arquivo,
      "Tempo em espera: %d ticks\n",
      produto->tempo_espera
   );

   fprintf(
      arquivo,
      "Falhas: %d\n",
      produto->falhas
   );

   escrever_historico(
      arquivo,
      produto
   );
}

void escrever_produto_na_fila(
   FILE *arquivo,
   const Produto *produto,
   const Simulacao *simulacao
) {
   int tempo_esperando =
      simulacao->tick_atual -
      produto->tick_criacao;

   fprintf(
      arquivo,
      "\n--- PRODUTO %d ---\n",
      produto->id
   );

   fprintf(
      arquivo,
      "Modelo: %s\n",
      simulacao->produto.nome
   );

   fprintf(
      arquivo,
      "Situacao: aguardando na fila\n"
   );

   fprintf(
      arquivo,
      "Criacao: tick %d\n",
      produto->tick_criacao
   );

   fprintf(
      arquivo,
      "Tempo aguardando: %d ticks\n",
      tempo_esperando
   );

   fprintf(
      arquivo,
      "Falhas: %d\n",
      produto->falhas
   );
}

void escrever_produto_ativo(
   FILE *arquivo,
   const Produto *produto,
   const Etapa *etapa,
   const Simulacao *simulacao
) {
   fprintf(
      arquivo,
      "\n--- PRODUTO %d ---\n",
      produto->id
   );

   fprintf(
      arquivo,
      "Modelo: %s\n",
      simulacao->produto.nome
   );

   fprintf(
      arquivo,
      "Situacao: em processamento\n"
   );

   fprintf(
      arquivo,
      "Criacao: tick %d\n",
      produto->tick_criacao
   );

   fprintf(
      arquivo,
      "Entrada na linha: tick %d\n",
      produto->tick_entrada
   );

   fprintf(
      arquivo,
      "Tempo em espera: %d ticks\n",
      produto->tempo_espera
   );

   fprintf(
      arquivo,
      "Etapa atual: %d - %s\n",
      etapa->id,
      etapa->nome
   );

   if (produto->atividade_atual != NULL) {
      fprintf(
         arquivo,
         "Atividade atual: %d - %s\n",
         produto->atividade_atual->id,
         produto->atividade_atual->nome
      );

      fprintf(
         arquivo,
         "Progresso da atividade: %d/%d ticks\n",
         produto->tempo_atividade,
         produto->atividade_atual->duracao
      );
   } else if (produto->aguardando_retorno) {
      fprintf(
         arquivo,
         "Atividade atual: controle de qualidade reprovado\n"
      );

      fprintf(
         arquivo,
         "Estado: aguardando vaga na etapa anterior\n"
      );
   } else {
      fprintf(
         arquivo,
         "Atividade atual: todas concluidas\n"
      );

      fprintf(
         arquivo,
         "Estado: aguardando vaga na proxima etapa\n"
      );
   }

   fprintf(
      arquivo,
      "Falhas: %d\n",
      produto->falhas
   );

   escrever_historico(
      arquivo,
      produto
   );
}

void escrever_estado_atual(
   FILE *arquivo,
   const Simulacao *simulacao
) {
   Produto *produto_atual;

   fprintf(
      arquivo,
      "\n--- PRODUTOS AGUARDANDO NA FILA ---\n"
   );

   produto_atual =
      simulacao->fila_entrada->inicio;

   if (produto_atual == NULL) {
      fprintf(
         arquivo,
         "Nenhum produto aguardando na fila.\n"
      );
   }

   while (produto_atual != NULL) {
      escrever_produto_na_fila(
         arquivo,
         produto_atual,
         simulacao
      );

      produto_atual =
         produto_atual->produto_prox;
   }

   fprintf(
      arquivo,
      "\n--- PRODUTOS ATIVOS NAS ETAPAS ---\n"
   );

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   int encontrou_produto_ativo = 0;

   while (etapa_atual != NULL) {
      produto_atual =
         etapa_atual->produtos;

      while (produto_atual != NULL) {
         encontrou_produto_ativo = 1;

         escrever_produto_ativo(
            arquivo,
            produto_atual,
            etapa_atual,
            simulacao
         );

         produto_atual =
            produto_atual->produto_prox;
      }

      etapa_atual =
         etapa_atual->etapa_prox;
   }

   if (!encontrou_produto_ativo) {
      fprintf(
         arquivo,
         "Nenhum produto ativo nas etapas.\n"
      );
   }
}

void escrever_analise_gargalo(
   FILE *arquivo,
   const Simulacao *simulacao
) {
   Etapa *gargalo = NULL;

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      if (
         gargalo == NULL ||
         etapa_atual->ticks_bloqueada >
            gargalo->ticks_bloqueada
      ) {
         gargalo = etapa_atual;
      }

      etapa_atual =
         etapa_atual->etapa_prox;
   }

   fprintf(
      arquivo,
      "\n--- ANALISE DE GARGALO ---\n"
   );

   if (
      gargalo == NULL ||
      gargalo->ticks_bloqueada == 0
   ) {
      fprintf(
         arquivo,
         "Nenhum gargalo identificado.\n"
      );

      return;
   }

   fprintf(
      arquivo,
      "Etapa mais bloqueada: %d - %s\n",
      gargalo->id,
      gargalo->nome
   );

   fprintf(
      arquivo,
      "Ticks bloqueada: %d\n",
      gargalo->ticks_bloqueada
   );

   fprintf(
      arquivo,
      "Ocupacao maxima: %d/%d\n",
      gargalo->ocupacao_maxima,
      gargalo->capacidade
   );
}

void gerar_relatorio(
   const Simulacao *simulacao,
   const char *arquivo_entrada,
   const char *arquivo_saida
) {
   if (
      simulacao == NULL ||
      arquivo_entrada == NULL ||
      arquivo_saida == NULL
   ) {
      return;
   }

   FILE *arquivo =
      fopen(arquivo_saida, "w");

   if (arquivo == NULL) {
      printf(
         "Erro ao criar o arquivo de relatorio: %s\n",
         arquivo_saida
      );

      return;
   }

   int produtos_finalizados =
      simulacao->produtos_concluidos +
      simulacao->descartados.quantidade;

   int produtos_faltantes =
      simulacao->produto.qtde_produzir -
      simulacao->produtos_concluidos;

   if (produtos_faltantes < 0) {
      produtos_faltantes = 0;
   }

   fprintf(
      arquivo,
      "=== METADADOS ===\n"
   );

   fprintf(
      arquivo,
      "id_simulacao: %s\n",
      simulacao->nome
   );

   fprintf(
      arquivo,
      "semente_utilizada: %d\n",
      simulacao->semente
   );

   fprintf(
      arquivo,
      "arquivo_entrada: %s\n",
      arquivo_entrada
   );

   fprintf(
      arquivo,
      "tick_fim: %d\n",
      simulacao->tick_atual
   );

   fprintf(
      arquivo,
      "produtos_criados: %d\n",
      simulacao->produtos_criados
   );

   fprintf(
      arquivo,
      "produtos_concluidos: %d\n",
      simulacao->produtos_concluidos
   );

   fprintf(
      arquivo,
      "produtos_descartados: %d\n",
      simulacao->descartados.quantidade
   );

   fprintf(
      arquivo,
      "produtos_finalizados: %d\n",
      produtos_finalizados
   );

   fprintf(
      arquivo,
      "tempo_medio_linha: %.2f\n",
      calcular_tempo_medio_total(simulacao)
   );

   fprintf(
      arquivo,
      "tempo_medio_espera: %.2f\n",
      calcular_tempo_medio_espera(simulacao)
   );

   fprintf(
      arquivo,
      "falhas_totais: %d\n",
      calcular_falhas_totais(simulacao)
   );

   fprintf(
      arquivo,
      "meta_alcancada: %s\n",
      simulacao->produtos_concluidos ==
         simulacao->produto.qtde_produzir
            ? "sim"
            : "nao"
   );

   fprintf(
      arquivo,
      "produtos_faltantes: %d\n",
      produtos_faltantes
   );

   fprintf(
      arquivo,
      "\n=== RELATORIO DA SIMULACAO %s ===\n",
      simulacao->nome
   );

   fprintf(
      arquivo,
      "\n--- RELATORIO DAS ETAPAS ---\n"
   );

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      fprintf(
         arquivo,
         "\nETAPA %d %s\n",
         etapa_atual->id,
         etapa_atual->nome
      );

      fprintf(
         arquivo,
         "Atividades: %d\n",
         etapa_atual->qtde_atividades
      );

      fprintf(
         arquivo,
         "Capacidade: %d\n",
         etapa_atual->capacidade
      );

      fprintf(
         arquivo,
         "Ocupacao final: %d\n",
         etapa_atual->ocupacao_atual
      );

      fprintf(
         arquivo,
         "Ocupacao maxima: %d/%d\n",
         etapa_atual->ocupacao_maxima,
         etapa_atual->capacidade
      );

      fprintf(
         arquivo,
         "Ticks bloqueada: %d\n",
         etapa_atual->ticks_bloqueada
      );

      fprintf(
         arquivo,
         "Taxa de falha: %.2f\n",
         etapa_atual->taxa_falha
      );

      fprintf(
         arquivo,
         "Quantidade de falhas: %d\n",
         etapa_atual->quantidade_falhas
      );

      Atividade *atividade_atual =
         etapa_atual->atividade;

      while (atividade_atual != NULL) {
         fprintf(
            arquivo,
            "  ATIVIDADE %d %s\n",
            atividade_atual->id,
            atividade_atual->nome
         );

         fprintf(
            arquivo,
            "  Tempo de execucao: %d\n",
            atividade_atual->duracao
         );

         fprintf(
            arquivo,
            "  Taxa de falha: %.2f\n",
            atividade_atual->taxa_falha
         );

         fprintf(
            arquivo,
            "  Quantidade de falhas: %d\n",
            atividade_atual->quantidade_falhas
         );

         atividade_atual =
            atividade_atual->atividade_prox;
      }

      etapa_atual =
         etapa_atual->etapa_prox;
   }

   escrever_analise_gargalo(
      arquivo,
      simulacao
   );

   fprintf(
      arquivo,
      "\n--- PRODUTOS CONCLUIDOS ---\n"
   );

   Produto *produto_atual =
      simulacao->lista_concluidos;

   if (produto_atual == NULL) {
      fprintf(
         arquivo,
         "Nenhum produto concluido.\n"
      );
   }

   while (produto_atual != NULL) {
      escrever_produto(
         arquivo,
         produto_atual,
         simulacao->produto.nome,
         "concluido"
      );

      produto_atual =
         produto_atual->produto_prox;
   }

   fprintf(
      arquivo,
      "\n--- PRODUTOS DESCARTADOS ---\n"
   );

   produto_atual =
      simulacao->descartados.topo;

   if (produto_atual == NULL) {
      fprintf(
         arquivo,
         "Nenhum produto descartado.\n"
      );
   }

   while (produto_atual != NULL) {
      escrever_produto(
         arquivo,
         produto_atual,
         simulacao->produto.nome,
         "descartado"
      );

      produto_atual =
         produto_atual->produto_prox;
   }

   escrever_estado_atual(
      arquivo,
      simulacao
   );

   fclose(arquivo);

   printf(
      "Relatorio gerado em: %s\n",
      arquivo_saida
   );
}

void gerar_relatorio_parcial(
   const Simulacao *simulacao,
   const char *arquivo_saida
) {
   if (
      simulacao == NULL ||
      arquivo_saida == NULL
   ) {
      return;
   }

   FILE *arquivo =
      fopen(arquivo_saida, "w");

   if (arquivo == NULL) {
      printf(
         "Erro ao criar o relatorio parcial: %s\n",
         arquivo_saida
      );

      return;
   }

   int produtos_finalizados =
      simulacao->produtos_concluidos +
      simulacao->descartados.quantidade;

   fprintf(
      arquivo,
      "=== RELATORIO PARCIAL ===\n"
   );

   fprintf(
      arquivo,
      "Simulacao: %s\n",
      simulacao->nome
   );

   fprintf(
      arquivo,
      "Tick atual: %d\n",
      simulacao->tick_atual
   );

   fprintf(
      arquivo,
      "Produtos previstos: %d\n",
      simulacao->produto.qtde_produzir
   );

   fprintf(
      arquivo,
      "Produtos criados: %d\n",
      simulacao->produtos_criados
   );

   fprintf(
      arquivo,
      "Produtos concluidos: %d\n",
      simulacao->produtos_concluidos
   );

   fprintf(
      arquivo,
      "Produtos descartados: %d\n",
      simulacao->descartados.quantidade
   );

   fprintf(
      arquivo,
      "Produtos finalizados: %d\n",
      produtos_finalizados
   );

   fprintf(
      arquivo,
      "Produtos na fila: %d\n",
      simulacao->fila_entrada->quantidade
   );

   fprintf(
      arquivo,
      "Falhas registradas: %d\n",
      calcular_falhas_totais(simulacao)
   );

   fprintf(
      arquivo,
      "\n--- ESTADO DAS ETAPAS ---\n"
   );

   Etapa *etapa_atual =
      simulacao->linha.inicio;

   while (etapa_atual != NULL) {
      fprintf(
         arquivo,
         "\nEtapa %d - %s\n",
         etapa_atual->id,
         etapa_atual->nome
      );

      fprintf(
         arquivo,
         "Ocupacao: %d/%d\n",
         etapa_atual->ocupacao_atual,
         etapa_atual->capacidade
      );

      fprintf(
         arquivo,
         "Ocupacao maxima: %d/%d\n",
         etapa_atual->ocupacao_maxima,
         etapa_atual->capacidade
      );

      fprintf(
         arquivo,
         "Ticks bloqueada: %d\n",
         etapa_atual->ticks_bloqueada
      );

      fprintf(
         arquivo,
         "Falhas: %d\n",
         etapa_atual->quantidade_falhas
      );

      etapa_atual =
         etapa_atual->etapa_prox;
   }

   escrever_analise_gargalo(
      arquivo,
      simulacao
   );

   escrever_estado_atual(
      arquivo,
      simulacao
   );

   fclose(arquivo);

   printf(
      "Relatorio parcial gerado em: %s\n",
      arquivo_saida
   );
}
