#include <stdio.h>

#include "relatorio.h"

static void linha_texto(FILE *arquivo, const char *rotulo, const char *valor) {
   fprintf(arquivo, "%s: %s\n", rotulo, valor);
}

static void linha_int(FILE *arquivo, const char *rotulo, int valor) {
   fprintf(arquivo, "%s: %d\n", rotulo, valor);
}

static void linha_float(FILE *arquivo, const char *rotulo, float valor) {
   fprintf(arquivo, "%s: %.2f\n", rotulo, valor);
}

static void linha_par(FILE *arquivo, const char *rotulo, int valor, int total) {
   fprintf(arquivo, "%s: %d/%d\n", rotulo, valor, total);
}

static void secao(FILE *arquivo, const char *titulo) {
   fprintf(arquivo, "\n--- %s ---\n", titulo);
}

static float dividir_seguro(int numerador, int denominador) {
   if (denominador == 0) {
      return 0.0f;
   }

   return (float) numerador / denominador;
}

static const char *id_simulacao_formatado(const Simulacao *simulacao) {
   static char id[60];

   snprintf(
      id,
      sizeof(id),
      "%s_%s",
      simulacao->nome,
      simulacao->timestamp
   );

   return id;
}

static int contar_produtos_ativos(const Simulacao *simulacao) {
   int ativos = 0;

   for (Etapa *etapa = simulacao->linha.inicio; etapa != NULL; etapa = etapa->etapa_prox) {
      ativos += etapa->ocupacao_atual;
   }

   return ativos;
}

static int calcular_falhas_lista(const Produto *produto) {
   int falhas = 0;

   while (produto != NULL) {
      falhas += produto->falhas;
      produto = produto->produto_prox;
   }

   return falhas;
}

int calcular_falhas_totais(const Simulacao *simulacao) {
   int falhas = calcular_falhas_lista(simulacao->lista_concluidos);
   falhas += calcular_falhas_lista(simulacao->descartados.topo);
   falhas += calcular_falhas_lista(simulacao->fila_entrada->inicio);

   for (Etapa *etapa = simulacao->linha.inicio; etapa != NULL; etapa = etapa->etapa_prox) {
      falhas += calcular_falhas_lista(etapa->produtos);
   }

   return falhas;
}

float calcular_tempo_medio_total(const Simulacao *simulacao) {
   if (simulacao->produtos_concluidos == 0) {
      return 0.0f;
   }

   int soma_tempos = 0;

   for (Produto *produto = simulacao->lista_concluidos; produto != NULL; produto = produto->produto_prox) {
      soma_tempos += produto->tempo_total;
   }

   return (float) soma_tempos / simulacao->produtos_concluidos;
}

float calcular_tempo_medio_espera(const Simulacao *simulacao) {
   if (simulacao->produtos_concluidos == 0) {
      return 0.0f;
   }

   int soma_espera = 0;

   for (Produto *produto = simulacao->lista_concluidos; produto != NULL; produto = produto->produto_prox) {
      soma_espera += produto->tempo_espera;
   }

   return (float) soma_espera / simulacao->produtos_concluidos;
}

static void escrever_historico_ordem(
   FILE *arquivo,
   const HistoricoNo *no,
   int *tem_grupo,
   int *etapa_atual,
   int *tentativa_atual,
   int *ticks_etapa_atual
) {
   if (no == NULL) {
      return;
   }

   escrever_historico_ordem(
      arquivo,
      no->prox,
      tem_grupo,
      etapa_atual,
      tentativa_atual,
      ticks_etapa_atual
   );

   if (!(*tem_grupo) || *etapa_atual != no->etapa_id || *tentativa_atual != no->tentativa) {
      if (*tem_grupo) {
         fprintf(arquivo, "Ticks na etapa: %d ticks\n", *ticks_etapa_atual);
      }

      fprintf(arquivo, "Etapa %d tentativa %d:\n", no->etapa_id, no->tentativa);
      *tem_grupo = 1;
      *etapa_atual = no->etapa_id;
      *tentativa_atual = no->tentativa;
   }

   fprintf(
      arquivo,
      "Atividade %d (%s) fila:%d inicio:%d fim:%d %s\n",
      no->atividade_id,
      no->atividade_nome,
      no->tick_fila,
      no->tick_inicio,
      no->tick_fim,
      no->sucesso ? "OK" : "FALHOU"
   );

   *ticks_etapa_atual = no->ticks_etapa;
}

static void escrever_historico(FILE *arquivo, const Produto *produto) {
   fprintf(arquivo, "Trajetoria:\n");

   if (produto->auditoria.topo == NULL) {
      fprintf(arquivo, "nenhuma atividade registrada\n");
      return;
   }

   int tem_grupo = 0;
   int etapa_atual = -1;
   int tentativa_atual = -1;
   int ticks_etapa_atual = 0;

   escrever_historico_ordem(
      arquivo,
      produto->auditoria.topo,
      &tem_grupo,
      &etapa_atual,
      &tentativa_atual,
      &ticks_etapa_atual
   );

   if (tem_grupo) {
      fprintf(arquivo, "Ticks na etapa: %d ticks\n", ticks_etapa_atual);
   }
}

static void escrever_produto(
   FILE *arquivo,
   const Produto *produto,
   const char *modelo,
   const char *situacao
) {
   fprintf(arquivo, "\n--- PRODUTO %d ---\n", produto->id);
   linha_texto(arquivo, "Modelo", modelo);
   linha_texto(arquivo, "Situacao", situacao);
   fprintf(arquivo, "Criacao: tick %d\n", produto->tick_criacao);
   fprintf(arquivo, "Entrada na linha: tick %d\n", produto->tick_entrada);
   fprintf(arquivo, "Saida da linha: tick %d\n", produto->tick_saida);
   fprintf(arquivo, "Tempo total: %d ticks\n", produto->tempo_total);
   fprintf(arquivo, "Tempo em espera: %d ticks\n", produto->tempo_espera);
   linha_int(arquivo, "Falhas", produto->falhas);
   escrever_historico(arquivo, produto);
}

static void escrever_produto_na_fila(
   FILE *arquivo,
   const Produto *produto,
   const Simulacao *simulacao
) {
   fprintf(arquivo, "\n--- PRODUTO %d ---\n", produto->id);
   linha_texto(arquivo, "Modelo", simulacao->produto.nome);
   linha_texto(arquivo, "Situacao", "aguardando na fila");
   fprintf(arquivo, "Criacao: tick %d\n", produto->tick_criacao);
   fprintf(arquivo, "Tempo aguardando: %d ticks\n", simulacao->tick_atual - produto->tick_criacao);
   linha_int(arquivo, "Falhas", produto->falhas);
}

static void escrever_produto_ativo(
   FILE *arquivo,
   const Produto *produto,
   const Etapa *etapa,
   const Simulacao *simulacao
) {
   fprintf(arquivo, "\n--- PRODUTO %d ---\n", produto->id);
   linha_texto(arquivo, "Modelo", simulacao->produto.nome);
   linha_texto(arquivo, "Situacao", "em processamento");
   fprintf(arquivo, "Criacao: tick %d\n", produto->tick_criacao);
   fprintf(arquivo, "Entrada na linha: tick %d\n", produto->tick_entrada);
   fprintf(arquivo, "Tempo em espera: %d ticks\n", produto->tempo_espera);
   fprintf(arquivo, "Etapa atual: %d - %s\n", etapa->id, etapa->nome);

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
      linha_texto(arquivo, "Atividade atual", "controle de qualidade reprovado");
      linha_texto(arquivo, "Estado", "aguardando vaga na etapa anterior");
   } else {
      linha_texto(arquivo, "Atividade atual", "todas concluidas");
      linha_texto(arquivo, "Estado", "aguardando vaga na proxima etapa");
   }

   linha_int(arquivo, "Falhas", produto->falhas);
   escrever_historico(arquivo, produto);
}

static void escrever_estado_atual(FILE *arquivo, const Simulacao *simulacao) {
   secao(arquivo, "PRODUTOS AGUARDANDO NA FILA");

   if (simulacao->fila_entrada->inicio == NULL) {
      fprintf(arquivo, "Nenhum produto aguardando na fila.\n");
   }

   for (Produto *produto = simulacao->fila_entrada->inicio; produto != NULL; produto = produto->produto_prox) {
      escrever_produto_na_fila(arquivo, produto, simulacao);
   }

   secao(arquivo, "PRODUTOS ATIVOS NAS ETAPAS");

   int encontrou_produto_ativo = 0;

   for (Etapa *etapa = simulacao->linha.inicio; etapa != NULL; etapa = etapa->etapa_prox) {
      for (Produto *produto = etapa->produtos; produto != NULL; produto = produto->produto_prox) {
         encontrou_produto_ativo = 1;
         escrever_produto_ativo(arquivo, produto, etapa, simulacao);
      }
   }

   if (!encontrou_produto_ativo) {
      fprintf(arquivo, "Nenhum produto ativo nas etapas.\n");
   }
}

static void escrever_analise_gargalo(FILE *arquivo, const Simulacao *simulacao) {
   Etapa *gargalo = NULL;

   for (Etapa *etapa = simulacao->linha.inicio; etapa != NULL; etapa = etapa->etapa_prox) {
      if (gargalo == NULL || etapa->ticks_bloqueada > gargalo->ticks_bloqueada) {
         gargalo = etapa;
      }
   }

   secao(arquivo, "ANALISE DE GARGALO");

   if (gargalo == NULL || gargalo->ticks_bloqueada == 0) {
      fprintf(arquivo, "Nenhum gargalo identificado.\n");
      return;
   }

   fprintf(arquivo, "Etapa mais bloqueada: %d - %s\n", gargalo->id, gargalo->nome);
   linha_int(arquivo, "Ticks bloqueada", gargalo->ticks_bloqueada);
   linha_par(arquivo, "Ocupacao maxima", gargalo->ocupacao_maxima, gargalo->capacidade);
}

static void escrever_relatorio_atividades(FILE *arquivo, const Simulacao *simulacao) {
   secao(arquivo, "RELATORIO DAS ATIVIDADES");

   for (Etapa *etapa = simulacao->linha.inicio; etapa != NULL; etapa = etapa->etapa_prox) {
      fprintf(arquivo, "\nETAPA %d %s:\n", etapa->id, etapa->nome);

      for (Atividade *atividade = etapa->atividade; atividade != NULL; atividade = atividade->atividade_prox) {
         fprintf(arquivo, "ATIVIDADE %d %s:\n", atividade->id, atividade->nome);
         linha_int(arquivo, "Capacidade", etapa->capacidade);
         linha_int(arquivo, "Vazao", atividade->produtos_processados);
         linha_int(arquivo, "Tempo de execucao", atividade->duracao);
         linha_float(
            arquivo,
            "Tempo medio em fila",
            dividir_seguro(atividade->soma_tempos_fila, atividade->produtos_processados)
         );
         linha_float(
            arquivo,
            "Tempo medio total",
            dividir_seguro(atividade->soma_tempos_totais, atividade->produtos_processados)
         );
         linha_int(arquivo, "Quantidade de falhas", atividade->quantidade_falhas);
      }
   }
}

static void escrever_relatorio_etapas(FILE *arquivo, const Simulacao *simulacao) {
   secao(arquivo, "RELATORIO DAS ETAPAS");

   for (Etapa *etapa = simulacao->linha.inicio; etapa != NULL; etapa = etapa->etapa_prox) {
      fprintf(arquivo, "\nETAPA %d %s\n", etapa->id, etapa->nome);
      linha_int(arquivo, "Atividades", etapa->qtde_atividades);
      linha_int(arquivo, "Capacidade", etapa->capacidade);
      linha_int(arquivo, "Ocupacao final", etapa->ocupacao_atual);
      linha_par(arquivo, "Ocupacao maxima", etapa->ocupacao_maxima, etapa->capacidade);
      linha_int(arquivo, "Ticks bloqueada", etapa->ticks_bloqueada);
      linha_float(arquivo, "Taxa de retorno/controle inicial", etapa->taxa_falha);
      linha_int(arquivo, "Quantidade de falhas", etapa->quantidade_falhas);
      linha_float(
         arquivo,
         "Falhas por produto",
         dividir_seguro(etapa->quantidade_falhas, etapa->produtos_processados)
      );
      linha_int(arquivo, "Tempo minimo", etapa->tempo_minimo);
      linha_float(arquivo, "Tempo medio", dividir_seguro(etapa->soma_tempos, etapa->produtos_processados));
      linha_int(arquivo, "Tempo maximo", etapa->tempo_maximo);
      linha_float(arquivo, "Tempo medio em fila", dividir_seguro(etapa->soma_tempos_fila, etapa->entradas_na_etapa));
   }
}

static void escrever_produtos_por_situacao(
   FILE *arquivo,
   Produto *produto,
   const char *modelo,
   const char *situacao,
   const char *mensagem_vazio
) {
   if (produto == NULL) {
      fprintf(arquivo, "%s\n", mensagem_vazio);
   }

   while (produto != NULL) {
      escrever_produto(arquivo, produto, modelo, situacao);
      produto = produto->produto_prox;
   }
}

static void escrever_metadados(
   FILE *arquivo,
   const Simulacao *simulacao,
   const char *arquivo_entrada,
   int produtos_finalizados,
   int produtos_faltantes
) {
   fprintf(arquivo, "=== METADADOS ===\n");
   linha_texto(arquivo, "id_simulacao", id_simulacao_formatado(simulacao));
   linha_int(arquivo, "semente_utilizada", simulacao->semente);
   linha_texto(arquivo, "arquivo_entrada", arquivo_entrada);
   linha_int(arquivo, "tick_fim", simulacao->tick_atual);
   linha_int(arquivo, "produtos_criados", simulacao->produtos_criados);
   linha_int(arquivo, "produtos_concluidos", simulacao->produtos_concluidos);
   linha_int(arquivo, "produtos_descartados", simulacao->descartados.quantidade);
   linha_int(arquivo, "produtos_finalizados", produtos_finalizados);
   linha_int(arquivo, "produtos_na_fila", simulacao->fila_entrada->quantidade);
   linha_int(arquivo, "produtos_ativos", contar_produtos_ativos(simulacao));
   linha_float(arquivo, "tempo_medio_linha", calcular_tempo_medio_total(simulacao));
   linha_float(arquivo, "tempo_medio_espera", calcular_tempo_medio_espera(simulacao));
   linha_int(arquivo, "falhas_totais", calcular_falhas_totais(simulacao));
   linha_texto(
      arquivo,
      "meta_alcancada",
      simulacao->produtos_concluidos == simulacao->produto.qtde_produzir ? "sim" : "nao"
   );
   linha_int(arquivo, "produtos_faltantes", produtos_faltantes);
}

void gerar_relatorio(
   const Simulacao *simulacao,
   const char *arquivo_entrada,
   const char *arquivo_saida
) {
   if (simulacao == NULL || arquivo_entrada == NULL || arquivo_saida == NULL) {
      return;
   }

   FILE *arquivo = fopen(arquivo_saida, "w");

   if (arquivo == NULL) {
      printf("Erro ao criar o arquivo de relatorio: %s\n", arquivo_saida);
      return;
   }

   int produtos_finalizados = simulacao->produtos_concluidos + simulacao->descartados.quantidade;
   int produtos_faltantes = simulacao->produto.qtde_produzir - simulacao->produtos_concluidos;

   if (produtos_faltantes < 0) {
      produtos_faltantes = 0;
   }

   escrever_metadados(
      arquivo,
      simulacao,
      arquivo_entrada,
      produtos_finalizados,
      produtos_faltantes
   );

   fprintf(arquivo, "\n=== RELATORIO DA SIMULACAO %s ===\n", id_simulacao_formatado(simulacao));
   escrever_relatorio_etapas(arquivo, simulacao);
   escrever_relatorio_atividades(arquivo, simulacao);
   escrever_analise_gargalo(arquivo, simulacao);

   secao(arquivo, "RELATORIO DOS PRODUTOS");
   secao(arquivo, "PRODUTOS CONCLUIDOS");
   escrever_produtos_por_situacao(
      arquivo,
      simulacao->lista_concluidos,
      simulacao->produto.nome,
      "concluido",
      "Nenhum produto concluido."
   );

   secao(arquivo, "PRODUTOS DESCARTADOS");
   escrever_produtos_por_situacao(
      arquivo,
      simulacao->descartados.topo,
      simulacao->produto.nome,
      "descartado",
      "Nenhum produto descartado."
   );

   escrever_estado_atual(arquivo, simulacao);
   fclose(arquivo);

   printf("Relatorio gerado em: %s\n", arquivo_saida);
}

void gerar_relatorio_parcial(const Simulacao *simulacao, const char *arquivo_saida) {
   if (simulacao == NULL || arquivo_saida == NULL) {
      return;
   }

   FILE *arquivo = fopen(arquivo_saida, "w");

   if (arquivo == NULL) {
      printf("Erro ao criar o relatorio parcial: %s\n", arquivo_saida);
      return;
   }

   int produtos_finalizados = simulacao->produtos_concluidos + simulacao->descartados.quantidade;
   int produtos_ativos = contar_produtos_ativos(simulacao);

   fprintf(arquivo, "=== RELATORIO PARCIAL ===\n");
   linha_texto(arquivo, "Simulacao", id_simulacao_formatado(simulacao));
   linha_int(arquivo, "Tick atual", simulacao->tick_atual);
   linha_int(arquivo, "Produtos previstos", simulacao->produto.qtde_produzir);
   linha_int(arquivo, "Produtos criados", simulacao->produtos_criados);
   linha_int(arquivo, "Produtos concluidos", simulacao->produtos_concluidos);
   linha_int(arquivo, "Produtos descartados", simulacao->descartados.quantidade);
   linha_int(arquivo, "Produtos finalizados", produtos_finalizados);
   linha_int(arquivo, "Produtos na fila", simulacao->fila_entrada->quantidade);
   linha_int(arquivo, "Produtos ativos", produtos_ativos);
   fprintf(
      arquivo,
      "Conferencia produtos: %d/%d\n",
      produtos_finalizados + simulacao->fila_entrada->quantidade + produtos_ativos,
      simulacao->produtos_criados
   );
   linha_int(arquivo, "Falhas registradas", calcular_falhas_totais(simulacao));

   secao(arquivo, "ESTADO DAS ETAPAS");

   for (Etapa *etapa = simulacao->linha.inicio; etapa != NULL; etapa = etapa->etapa_prox) {
      fprintf(arquivo, "\nEtapa %d - %s\n", etapa->id, etapa->nome);
      linha_par(arquivo, "Ocupacao", etapa->ocupacao_atual, etapa->capacidade);
      linha_par(arquivo, "Ocupacao maxima", etapa->ocupacao_maxima, etapa->capacidade);
      linha_int(arquivo, "Ticks bloqueada", etapa->ticks_bloqueada);
      linha_int(arquivo, "Falhas", etapa->quantidade_falhas);
      linha_float(arquivo, "Tempo medio", dividir_seguro(etapa->soma_tempos, etapa->produtos_processados));
      linha_float(arquivo, "Tempo medio em fila", dividir_seguro(etapa->soma_tempos_fila, etapa->entradas_na_etapa));
   }

   escrever_analise_gargalo(arquivo, simulacao);
   escrever_estado_atual(arquivo, simulacao);
   fclose(arquivo);

   printf("Relatorio parcial gerado em: %s\n", arquivo_saida);
}
