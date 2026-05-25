#ifndef PRODUTO_H
#define PRODUTO_H

typedef struct Produto {
   int id;
   int falhas;
   int tempo_total;
} Produto;

Produto* criar_produto(int id);
void descartar_produto(Produto* Produto);

#endif
