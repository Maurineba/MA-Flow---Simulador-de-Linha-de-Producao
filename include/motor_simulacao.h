#ifndef MOTOR_SIMULACAO_H
#define MOTOR_SIMULACAO_H

#include "simulacao.h"

void gerar_produtos(Simulacao *simulacao);
void inserir_produtos_primeira_etapa(Simulacao *simulacao);
void processar_atividades(Simulacao *simulacao);
void movimentar_produtos(Simulacao *simulacao);
void executar_simulacao(Simulacao *simulacao, const char *arquivo_relatorio_parcial);
void atualizar_metricas_gargalo(Simulacao *simulacao);

#endif
