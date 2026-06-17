#ifndef ARVORE_H
#define ARVORE_H

#include "produto.h"

void inserir_produto_arvore(
   Produto **raiz,
   Produto *produto
);

Produto *buscar_produto_arvore(
   Produto *raiz,
   int id
);

Produto *remover_produto_arvore(
   Produto *raiz,
   int id
);

void imprimir_produtos_ativos(
   const Produto *raiz
);

#endif
