#ifndef PILHA_H
#define PILHA_H

#include "simulacao.h"

void empilhar_historico(Produto *produto, int atividade_id);
int desempilhar_historico(Produto *produto);
void liberar_historico(Produto *produto);

void empilhar_descarte(
   PilhaDescarte *pilha,
   Produto *produto
);

Produto *desempilhar_descarte(PilhaDescarte *pilha);
void liberar_pilha_descarte(PilhaDescarte *pilha);

#endif
