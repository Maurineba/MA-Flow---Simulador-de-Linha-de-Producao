#include <stdio.h>

#include "parser.h"
#include "motor_simulacao.h"
#include "relatorio.h"

int main(int argc, char *argv[]) {
   if (argc < 2) {
      printf(
         "Uso: %s <arquivo_entrada>\n",
         argv[0]
      );

      return 1;
   }

   const char *arquivo_entrada = argv[1];

   const char *arquivo_saida =
      "output/relatorio_producao.txt";

   const char *arquivo_relatorio_parcial =
      "output/relatorio_parcial.txt";

   Simulacao *simulacao =
      ler_arquivo(arquivo_entrada);

   if (simulacao == NULL) {
      printf(
         "Nao foi possivel carregar a simulacao: %s\n",
         arquivo_entrada
      );

      return 1;
   }

   executar_simulacao(
      simulacao,
      arquivo_relatorio_parcial
   );

   gerar_relatorio(
      simulacao,
      arquivo_entrada,
      arquivo_saida
   );

   liberar_simulacao(simulacao);

   return 0;
}
