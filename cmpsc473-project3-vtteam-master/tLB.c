#include "tLB.h"
#include <stdlib.h>

void memoryaccess(int lrusize)
{
    lrutlb = (struct lru_struct *) malloc (lrusize *  sizeof(struct lru_struct));

    lru_init(lrutlb, TLB_SIZE);

    tlb = (struct tlb_struct *) malloc (TLB_SIZE * sizeof(struct tlb_struct));

    tlb_init();
}

void tlb_init()
{
   int i = 0;
	
   while(i < TLB_SIZE)
   {
	tlb[i].valid = 0;
	i++;
   } 
}

int tlb_access(int vpn)
{
   tlb_total_access++;

   int flag = 0;
   uint32_t chosen_tag = vpn; 
   int chosen_index = tlb_get_tag(chosen_tag);
   
   if(chosen_index >= 0 && tlb[chosen_index].valid == 1)
   {
	lru_update(lrutlb, TLB_SIZE, chosen_index);
        flag = 1;
        tlb_total_hit++;
   }
  
   if(flag == 0)
	tlb_add(chosen_tag);

   return flag;
}

int tlb_get_tag(int tag)
{
   int i = 0;

   while(i < TLB_SIZE)
   {
	if(tlb[i].tag == tag)
            return i;
	i++;   
   }

   return -1;
}

void tlb_add(int tag)
{
   int index = lru_lookup(lrutlb, TLB_SIZE);

   tlb[index].valid = 1;
   tlb[index].tag = tag;
}

