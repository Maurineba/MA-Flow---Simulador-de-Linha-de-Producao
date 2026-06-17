#ifndef SIMULACAO_H
#define SIMULACAO_H

#include "fila.h"

typedef struct Atividade {
   int id;
   int duracao;
   int quantidade_falhas;
   int produtos_processados;
   int soma_tempos_fila;
   int soma_tempos_totais;
   float taxa_falha;
   char nome[30];

   struct Atividade *atividade_prox;
} Atividade;

typedef struct Etapa {
   int id;
   int qtde_atividades;
   int capacidade;
   int ocupacao_atual;
   int ocupacao_maxima;
   int quantidade_falhas;
   int ticks_bloqueada;
   int produtos_processados;
   int soma_tempos;
   int tempo_minimo;
   int tempo_maximo;
   int soma_tempos_fila;
   int entradas_na_etapa;
   float taxa_falha;
   char nome[30];

   Atividade *atividade;
   Produto *produtos;

   struct Etapa *etapa_ant;
   struct Etapa *etapa_prox;
} Etapa;

typedef struct LinhaProducao {
   int qtde_etapas;

   Etapa *inicio;
   Etapa *fim;

}  LinhaProducao;

typedef struct ProdutoConfig {
   int qtde_produzir;
   int taxa_por_seg;
   char nome[30];
} ProdutoConfig;

typedef struct PilhaDescarte {
   Produto *topo;
   int quantidade;

} PilhaDescarte;

typedef struct Simulacao {
   char nome[30];
   char timestamp[20];
   int semente;
   int tempo_limite;

   int tick_atual;
   int produtos_criados;
   int proximo_id_produto;
   int produtos_concluidos;

   ProdutoConfig produto;
   LinhaProducao linha;
   FilaProduto *fila_entrada;
   Produto *lista_concluidos;

   PilhaDescarte descartados;
   Produto *raiz_produtos_ativos;

} Simulacao;

Simulacao *criar_simulacao();
void liberar_simulacao(Simulacao *simulacao);

#endif
