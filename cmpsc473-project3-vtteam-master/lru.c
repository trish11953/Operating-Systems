#include "lRU.h"

// LRU init function to initialize an lru with initial values being 0
void lru_init(struct lru_struct *lru, int size)
{
   int i = 0;
   while(i < size)
   {
	lru[i].data = 0;
	i++;	
   } 
}

// LRU update function to update the lru function by traversing it in decreasing order
void lru_update(struct lru_struct *lru, int size, int index)
{
    lru_decrement(lru, size);
    lru[index].data = (size - 1);
}

int lru_lookup(struct lru_struct *lru,	int size)
{
    int oldest = lru[0].data;
    int oldestindex = 0;
    int i = 0;

    while(i < size)
    {
	if(lru[i].data <= oldest)
        {
           oldest = lru[i].data;
           oldestindex = i;
        }
        i++;
    }

    return oldestindex;
} 

void lru_decrement(struct lru_struct *lru, int size)
{
    int i = 0;

    while(i < size)
    {
	(lru[i].data)--;
	i++;
    }	
}
