#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "etapa.h"
#include "atividade.h"

#define TAM_LINHA 256
#define TAM_CHAVE 30

Simulacao *ler_arquivo(const char *path) {
   FILE *arquivo = fopen(path, "r");

   if (arquivo == NULL) {
      printf("\nErro ao abrir o arquivo de entrada.\n");
      printf("Caminho informado: %s\n", path);
      printf("Verifique se o arquivo existe e se o caminho esta correto.\n");
      return NULL;
   }

   Simulacao *simulacao = criar_simulacao();

   if (simulacao == NULL) {
      printf("\nErro ao iniciar a simulacao.\n");
      printf("Nao foi possivel alocar memoria para a estrutura Simulacao.\n");
      fclose(arquivo);
      return NULL;
   }

   char linha[TAM_LINHA];
   char chave[TAM_CHAVE];

   Etapa *etapa_atual = NULL;

   int etapas_lidas = 0;
   int atividades_lidas_etapa = 0;

   while (fgets(linha, sizeof(linha), arquivo) != NULL) {
      if (sscanf(linha, "%29s", chave) != 1) {
         continue;
      }

      if (strcmp(chave, "SIMULADOR") == 0) {
         continue;
      }

      if (strcmp(chave, "SIMULACAO") == 0) {
         if (sscanf(linha, "%*s %29s %d %d",
            simulacao->nome,
            &simulacao->semente,
            &simulacao->tempo_limite) != 3) {

            printf("\nErro ao ler a configuracao da simulacao.\n");
            printf("Formato esperado:\n");
            printf("SIMULACAO <nome> <semente> <tempo_limite>\n");
            printf("Linha recebida: %s", linha);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }
      }

      else if (
         strcmp(chave, "PRODUTO") == 0 ||
         strcmp(chave, "PRODUTOS") == 0
      ) {
         if (sscanf(linha, "%*s %d %d %29s",
            &simulacao->produto.qtde_produzir,
            &simulacao->produto.taxa_por_seg,
            simulacao->produto.nome) != 3) {

            printf("\nErro ao ler a configuracao do produto.\n");
            printf("Formato esperado:\n");
            printf("PRODUTO ou PRODUTOS <quantidade> <taxa_por_segundo> <nome>\n");
            printf("Linha recebida: %s", linha);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }
      }

      else if (strcmp(chave, "LINHA_PRODUCAO") == 0) {
         if (sscanf(linha, "%*s %d",
            &simulacao->linha.qtde_etapas) != 1) {

            printf("\nErro ao ler a configuracao da linha de producao.\n");
            printf("Formato esperado:\n");
            printf("LINHA_PRODUCAO <quantidade_de_etapas>\n");
            printf("Linha recebida: %s", linha);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }
      }

      else if (strcmp(chave, "ETAPA") == 0) {
         // valida a etapa anteior antes de criar uma nova
         if (etapa_atual != NULL && atividades_lidas_etapa != etapa_atual->qtde_atividades) {

            printf("\nErro na quantidade de atividades da etapa.\n");
            printf("Etapa: %d - %s\n",
               etapa_atual->id,
               etapa_atual->nome);
            printf("Quantidade declarada: %d\n",
               etapa_atual->qtde_atividades);
            printf("Quantidade encontrada: %d\n",
               atividades_lidas_etapa);
            printf("Verifique as atividades informadas antes da proxima ETAPA.\n");

            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }

         etapa_atual = criar_etapa_por_linha(linha);
         if (etapa_atual == NULL) {
            printf("\nErro ao ler uma etapa da linha de producao.\n");
            printf("Formato esperado:\n");
            printf("ETAPA <id> <quantidade_atividades> <capacidade> <taxa_falha> <nome>\n");
            printf("Linha recebida: %s", linha);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }
         if (existe_etapa_com_id(&simulacao->linha, etapa_atual->id)) {
            printf("\nErro ao ler uma etapa da linha de producao.\n");
            printf("Foram encontradas duas etapas com o mesmo ID.\n");
            printf("ID duplicado: %d\n", etapa_atual->id);
            printf("Linha com problema: %s", linha);
            free(etapa_atual);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }

         inserir_etapa(&simulacao->linha, etapa_atual);

         etapas_lidas++;
         atividades_lidas_etapa = 0;
      }

      else if (strcmp(chave, "ATIVIDADE") == 0) {
         if (etapa_atual == NULL) {
            printf("\nErro ao ler uma atividade.\n");
            printf("Uma ATIVIDADE foi encontrada antes de qualquer ETAPA.\n");
            printf("Cada atividade deve estar abaixo da etapa a que pertence.\n");
            printf("Linha com problema: %s", linha);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }

         Atividade *atividade = criar_atividade_por_linha(linha);
         if (atividade == NULL) {
            printf("\nErro ao ler uma atividade da etapa %d.\n",
               etapa_atual->id);
            printf("Formato esperado:\n");
            printf("ATIVIDADE <id> <duracao> <taxa_falha> <nome>\n");
            printf("Linha recebida: %s", linha);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }

         if (existe_atividade_com_id(etapa_atual, atividade->id)) {
            printf("\nErro ao ler uma atividade.\n");
            printf("A etapa %d possui duas atividades com o mesmo ID.\n",
               etapa_atual->id);
            printf("ID duplicado: %d\n", atividade->id);
            printf("Linha com problema: %s", linha);

            free(atividade);
            liberar_simulacao(simulacao);
            fclose(arquivo);
            return NULL;
         }

         inserir_atividade(etapa_atual, atividade);

         atividades_lidas_etapa++;
      }

      else {
         printf("\nErro ao ler o arquivo de entrada.\n");
         printf("Palavra-chave desconhecida: %s\n", chave);
         printf("Palavras-chave permitidas:\n");
         printf("SIMULADOR, SIMULACAO, PRODUTO/PRODUTOS, LINHA_PRODUCAO, ETAPA e ATIVIDADE.\n");
         printf("Linha com problema: %s", linha);
         liberar_simulacao(simulacao);
         fclose(arquivo);
         return NULL;
      }
   }

   // verifa a ultima etapa
   if (etapa_atual != NULL && atividades_lidas_etapa != etapa_atual->qtde_atividades) {

      printf("\nErro na quantidade de atividades da ultima etapa.\n");
      printf("Etapa: %d - %s\n",
         etapa_atual->id,
         etapa_atual->nome);
      printf("Quantidade declarada: %d\n",
         etapa_atual->qtde_atividades);
      printf("Quantidade encontrada: %d\n",
         atividades_lidas_etapa);
      printf("O arquivo terminou antes que todas as atividades fossem lidas.\n");

      liberar_simulacao(simulacao);
      fclose(arquivo);
      return NULL;
   }
   // validando quantidade total de etapas depois do fim da leitura
   if (etapas_lidas != simulacao->linha.qtde_etapas) {
      printf("\nErro na quantidade de etapas da linha de producao.\n");
      printf("Quantidade declarada: %d\n",
         simulacao->linha.qtde_etapas);
      printf("Quantidade encontrada: %d\n",
         etapas_lidas);
      printf("Verifique o valor de LINHA_PRODUCAO e as etapas informadas.\n");

      liberar_simulacao(simulacao);
      fclose(arquivo);
      return NULL;
   }

   fclose(arquivo);
   return simulacao;
}
