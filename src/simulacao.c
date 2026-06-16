#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulacao.h"

char P_CHAVES[6][30] = ["SIMULACAO", "PRODUTO", "LINHA_PRODUCAO", "ETAPA", "ATIVIDADE"];

char[] extrair(FILE *aq)

Simulacao *ler_arquivo(char *path) {
   FILE *arquivo;

   arquivo = fopen("%s", "r");
   if (arquivo == NULL){
      printf("Erro ao abrir arquivo");
      return;
   }
   printf("Lendo arquivo...");



   fclose(arquivo);
}



Simulacao *criar_simulacao() {

}
