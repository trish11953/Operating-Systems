#include "pagetable.h"
#include <math.h>

int totallevel2pages = 0;
int totallevel3pages = 0;

int accessinglevel1(int N1, int N2, int N3, int va, uint32_t level1mask, uint32_t level2mask,uint32_t level3mask, uint32_t maskoffset)
{  
   
   uint32_t vpn = (va >> (32 - N1)) & level1mask;
   uint32_t offset = va & maskoffset;
   int flag = 0;
   int level2size = pow(2,N2);
   int level3size = pow(2,N3);
   if(!PTlevel1[vpn].valid)
   {   
	addlevel1entry(N2,N3,vpn,vpn, level2size);
   }
   else if(PTlevel1[vpn].valid && N2 == 0) 
   {
	flag = 1;
   } 
   else if(PTlevel1[vpn].valid && N2 != 0)
   {    
                
	flag = accessinglevel2(N1, N2, N3, va, PTlevel1[vpn].ppn, level2mask, level3mask, maskoffset);
   }

   return flag;
}

void addlevel1entry(int N2, int N3, int vpn,  int PTLEVEL2SIZE,int PTLEVEL3SIZE)
{   
    //uint32_t offset = (vpn >> (32 - N1 - N2)) & level2mask;
    PTlevel1[vpn].valid = 1;   
   if(N2 != 0)
   {
	if(totallevel2pages == 0)
   	{  
	   PTlevel2 = (struct pagetable *) malloc (PTLEVEL2SIZE * sizeof(struct pagetable));
	   PTlevel1[vpn].ppn = 0; 
	}
	else
	{       free(PTlevel2);
           	PTlevel2 = (struct pagetable *) malloc (((totallevel2pages +1) * PTLEVEL2SIZE));
		PTlevel1[vpn].ppn = totallevel2pages * PTLEVEL2SIZE;
		//addlevel2entry(N1, N2, N3, PTlevel1[vpn].ppn, offset, PTLEVEL3SIZE, level3mask);
	}
        
        //addlevel2entry(N1, N2, N3, PTlevel1[vpn].ppn, offset, PTLEVEL3SIZE, level3mask);
	totallevel2pages++;
   }
   
}

int accessinglevel2(int N1, int N2, int N3, int va, uint32_t level_1_ppn, uint32_t level2mask,uint32_t level3mask, uint32_t maskoffset)
{
   uint32_t vpn_offset = (va >> (32 - N1 - N2)) & level2mask;
   uint32_t offset = va & maskoffset;
   uint32_t vpn = level_1_ppn + vpn_offset;
   int flag = 0;
   int level3size = pow(2, N3);

   if(!PTlevel2[vpn].valid)
   {
    	addlevel2entry(N3,vpn,level3size);
   }
   else if(PTlevel2[vpn].valid && N3 == 0)
   {
	flag = 1;
   }
   else if(PTlevel2[vpn].valid && N3 != 0) 	
   {
	flag = accessinglevel3(N1, N2, N3, va, PTlevel2[vpn].ppn, level3mask, maskoffset);
   }

return flag;

}

void addlevel2entry(int N3, int vpn, int PTLEVEL3SIZE)
{
   //uint32_t offset = (vpn >> (32 - N1 - N2 - N3)) & level3mask;
   PTlevel2[vpn].valid = 1;
   if(N3 != 0)
   {
    	if(totallevel3pages == 0)
        {
           PTlevel3 = (struct pagetable *) malloc (PTLEVEL3SIZE * sizeof(struct pagetable));
           PTlevel2[vpn].ppn = 0;
        }
	else
	{  
           	PTlevel3 = (struct pagetable *) malloc (((totallevel3pages +1) * PTLEVEL3SIZE));
		PTlevel2[vpn].ppn = totallevel3pages * PTLEVEL3SIZE;
                //addlevel3entry(PTlevel2[vpn].ppn + offset);        
	}
	//addlevel3entry(PTlevel2[vpn].ppn + offset);
        totallevel3pages++;
   }

} 

int accessinglevel3(int N1, int N2, int N3, int va, uint32_t level_2_ppn, uint32_t level3mask, uint32_t maskoffset)
{
   uint32_t vpn_offset = (va >> (32 - N1 - N2 - N3)) & level3mask;
   uint32_t offset = va & maskoffset;
   uint32_t vpn = level_2_ppn + vpn_offset;
   int flag = 0;
    printf("adding to level3");
   if(!PTlevel3[vpn].valid)
   {
    	addlevel3entry(vpn);
   }
   else
   {
        flag = 1;
   }
 return flag;
}

void addlevel3entry(int vpn)
{  
   printf("Entering level3");
   PTlevel3[vpn].valid = 1;
}
