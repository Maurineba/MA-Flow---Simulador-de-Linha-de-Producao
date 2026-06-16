#include <stdio.h>

#include "parser.h"
#include "motor_simulacao.h"

void imprimir_historico(const Produto *produto) {
   HistoricoNo *no_atual = produto->historico.topo;

   printf("Historico do produto %d: ", produto->id);

   while (no_atual != NULL) {
      printf("%d ", no_atual->atividade_id);
      no_atual = no_atual->prox;
   }

   printf("\n");
}

int main() {
   Simulacao *simulacao = ler_arquivo("input/test.txt");

   if (simulacao == NULL) {
      printf("Nao foi possivel carregar a simulacao.\n");
      return 1;
   }

   while (
      simulacao->produtos_concluidos <
         simulacao->produto.qtde_produzir &&
      simulacao->tick_atual <
         simulacao->tempo_limite
   ) {
      printf("\n=== TICK %d ===\n", simulacao->tick_atual);

      processar_atividades(simulacao);
      movimentar_produtos(simulacao);
      gerar_produtos(simulacao);
      inserir_produtos_primeira_etapa(simulacao);

      simulacao->tick_atual++;
   }

   Produto *produto = simulacao->lista_concluidos;

   if (produto != NULL) {
      printf("\nProduto concluido: %d\n", produto->id);

      printf("Quantidade no historico: %d\n",
         produto->historico.quantidade);

      imprimir_historico(produto);
   }

   liberar_simulacao(simulacao);

   return 0;
}
