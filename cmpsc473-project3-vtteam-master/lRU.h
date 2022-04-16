#include <stdio.h>

struct lru_struct
{
   int data;
} *lrutlb, *lrulevel1, *lrulevel2, *lrulevel3;

void lru_init(struct lru_struct *lru, int size);
void lru_update(struct lru_struct *lru, int size, int index);
int lru_lookup(struct lru_struct *lru, int size);
void lru_decrement(struct lru_struct *lru, int size);
