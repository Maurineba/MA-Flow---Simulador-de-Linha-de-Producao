#ifndef SIMULACAO_H
#define SIMULACAO_H

typedef struct Atividade {
   int id;
   int duracao;
   float taxa_falha;
   char nome[30];

   struct Atividade *atividade_prox;

} Atividade;

typedef struct Etapa {
   int id;
   int qtde_atividades; //qtde = quantidade
   int capacidade;
   int ocupacao_atual;
   float taxa_falha;
   char nome[30];

   Atividade *atividade;

   struct Etapa *etapa_ant;
   struct Etapa *etapa_prox;

}  Etapa;

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

typedef struct Simulacao {
   char nome[30];
   int semente;
   int tempo_limite;

   ProdutoConfig produto;
   LinhaProducao linha;
} Simulacao;

Simulacao *criar_simulacao();
Simulacao *ler_arquivo(const char *path);
void liberar_simulacao(Simulacao *simulacao);

#endif
