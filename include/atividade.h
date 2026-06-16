#ifndef ATIVIDADE_H
#define ATIVIDADE_H

#include "simulacao.h"

Atividade *criar_atividade_por_linha(const char *linha);
void inserir_atividade(Etapa *etapa, Atividade *atividade);
int existe_atividade_com_id(const Etapa *etapa, int id);

#endif
