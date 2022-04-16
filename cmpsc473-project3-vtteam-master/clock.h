#include <stdio.h>

struct clk_struct
{
   int data;
} *clktlb, *clklevel1, *clklevel2, *clklevel3;

void clk_init(struct clk_struct *clk, int size);
void clk_update(struct clk_struct *clk, int size, int index);
int clk_lookup(struct clk_struct *clk, int size);
void clk_decrement(struct clk_struct *clk, int size);


