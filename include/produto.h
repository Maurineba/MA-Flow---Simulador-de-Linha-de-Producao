#ifndef PRODUTO_H
#define PRODUTO_H

struct Etapa;
struct Atividade;

typedef struct HistoricoNo {
   int atividade_id;
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
   struct Etapa *etapa_atual;
   struct Atividade *atividade_atual;

   PilhaHistorico historico;

   struct Produto *produto_prox;

} Produto;

Produto *criar_produto(int id, int tick_criacao);
void liberar_produto(Produto *produto);

#endif
