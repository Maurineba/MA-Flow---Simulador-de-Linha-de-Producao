#include <stdio.h>
#include "produto.h"

int main() {
   Produto* p = criar_produto(1);
   if (p == NULL) {
      printf("erro ao criar produto");
      return 1;
   }
   
   printf("sucesso. id do produto: %d", p->id);
   descartar_produto(p);

   return 0;
}
