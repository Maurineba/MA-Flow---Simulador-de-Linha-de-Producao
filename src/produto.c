#include <stdio.h>
#include <stdlib.h>

#include "produto.h"
#include "pilha.h"

Produto *criar_produto(int id, int tick_criacao) {
   Produto *novo_produto = malloc(sizeof(Produto));

   if (novo_produto == NULL) {
      printf("Erro ao alocar memoria para o produto.\n");
      return NULL;
   }

   novo_produto->id = id;
   novo_produto->falhas = 0;

   novo_produto->tick_criacao = tick_criacao;
   novo_produto->tick_entrada = -1;
   novo_produto->tick_saida = -1;

   novo_produto->tempo_total = 0;
   novo_produto->tempo_espera = 0;

   novo_produto->tempo_atividade = 0;
   novo_produto->etapa_atual = NULL;
   novo_produto->atividade_atual = NULL;

   novo_produto->historico.topo = NULL;
   novo_produto->historico.quantidade = 0;

   novo_produto->produto_prox = NULL;
   novo_produto->arvore_esq = NULL;
   novo_produto->arvore_dir = NULL;
   novo_produto->aguardando_retorno = 0;

   return novo_produto;
}

void liberar_produto(Produto *produto) {
   if (produto == NULL) {
      return;
   }

   liberar_historico(produto);
   free(produto);
}
