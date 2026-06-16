#ifndef ETAPA_H
#define ETAPA_H

#include "simulacao.h"

Etapa *criar_etapa_por_linha(const char *linha);
void inserir_etapa(LinhaProducao *linha, Etapa *etapa);
int existe_etapa_com_id(const LinhaProducao *linha, int id);
void inserir_produto_etapa(Etapa *etapa, Produto *produto);
Produto *remover_produto_etapa(Etapa *etapa, Produto *produto);

#endif
