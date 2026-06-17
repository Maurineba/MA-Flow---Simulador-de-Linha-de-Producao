#ifndef PILHA_H
#define PILHA_H

#include "simulacao.h"

void empilhar_historico(
   Produto *produto,
   int etapa_id,
   int atividade_id,
   const char *atividade_nome,
   int tentativa,
   int tick_fila,
   int tick_inicio,
   int tick_fim,
   int ticks_etapa,
   int sucesso
);

void empilhar_auditoria(
   Produto *produto,
   int etapa_id,
   int atividade_id,
   const char *atividade_nome,
   int tentativa,
   int tick_fila,
   int tick_inicio,
   int tick_fim,
   int ticks_etapa,
   int sucesso
);

int desempilhar_historico(Produto *produto);
void liberar_historico(Produto *produto);

void empilhar_descarte(PilhaDescarte *pilha, Produto *produto);

Produto *desempilhar_descarte(PilhaDescarte *pilha);
void liberar_pilha_descarte(PilhaDescarte *pilha);
void remover_historico_etapa( Produto *produto, int etapa_id );

#endif
