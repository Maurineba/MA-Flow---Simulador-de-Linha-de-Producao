#ifndef RELATORIO_H
#define RELATORIO_H

#include "simulacao.h"

void gerar_relatorio(
   const Simulacao *simulacao,
   const char *arquivo_entrada,
   const char *arquivo_saida
);

void gerar_relatorio_parcial(
   const Simulacao *simulacao,
   const char *arquivo_saida
);

#endif
