#include "clock.h"

// clock init function to initialize an clock with initial values being 0
void clock_init(struct clk_struct *clk, int size)
{
   int i = 0;
   while(i < size)
   {
    	lru[i].data = 0;
        i++;
   }
}

void clock_update(struct clk_struct *clk, int size, int index)
{
    clock_increment(clk, size);
    clock[index].data = (size - 1);
}

int clock_lookup(struct clk_struct *clk,  int size)
{
    int curr_time = clk[0].data;
    int index = 0;
    int i = 0;

    while(i < size)
    {
     	if(clk[i].data <= curr_time)
        {
           curr_time = clk[i].data;
           index = i;
        }
	i++;
    }

    return oldestindex;
}

void clock_increment(struct clk_struct *clk, int size)
{
    int i = 0;

    while(i < size)
    {
     	(clk[i].data)--;
        i++;
    }
}

