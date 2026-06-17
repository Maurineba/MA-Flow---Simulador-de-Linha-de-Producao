#ifndef PRODUTO_H
#define PRODUTO_H

struct Etapa;
struct Atividade;

typedef struct HistoricoNo {
   int etapa_id;
   int atividade_id;
   char atividade_nome[30];
   int tentativa;
   int tick_fila;
   int tick_inicio;
   int tick_fim;
   int ticks_etapa;
   int sucesso;

   struct HistoricoNo *prox;

} HistoricoNo;

typedef struct PilhaHistorico {
   HistoricoNo *topo;
   int quantidade;

} PilhaHistorico;

typedef struct Produto {
   int id;
   int falhas;

   int tick_criacao;
   int tick_entrada;
   int tick_saida;

   int tempo_total;
   int tempo_espera;

   int tempo_atividade;
   int aguardando_retorno;
   int tick_entrada_etapa;
   int tick_entrada_atividade;
   int tick_inicio_atividade;
   int tentativa_etapa;
   int etapa_tentativa_id;
   int controle_qualidade_verificado;
   int etapa_metricas_registradas;

   struct Etapa *etapa_atual;
   struct Atividade *atividade_atual;

   PilhaHistorico historico;
   PilhaHistorico auditoria;

   struct Produto *produto_prox;

   struct Produto *arvore_esq;
   struct Produto *arvore_dir;
} Produto;

Produto *criar_produto(int id, int tick_criacao);
void liberar_produto(Produto *produto);

#endif
