#include <stdio.h>
#include <stdint.h>
#include "lRU.h"
#define TLB_SIZE 16
   
struct tlb_struct
{
   uint32_t ppn;
   uint32_t tag;
   int valid;
} *tlb;

void memoryaccess(int lrusize);
void tlb_init();
int tlb_access(int vpn);
int tlb_get_tag(int tag);
void tlb_add(int tag);


//might not need
int tlb_total_access;
int tlb_total_hit;
