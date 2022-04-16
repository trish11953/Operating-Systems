#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
/*
int DRAM_SIZE = 0;
int PTLEVEL1SIZE = 0;
int PTLEVEL2SIZE = 0;
int PTLEVEL3SIZE = 0;
int OFFSETSIZE = 0;
uint32_t level1mask = 1;
uint32_t level2mask = 1;
uint32_t level3mask = 1;
uint32_t maskoffset = 1;
*/

struct pagetable
{
  uint32_t ppn;
  int valid;
}*PTlevel1, *PTlevel2, *PTlevel3;

int accessinglevel1(int N1, int N2, int N3, int va, uint32_t level1mask, uint32_t level2mask,uint32_t level3mask, uint32_t maskoffset);
void addlevel1entry(int N2, int N3, int vpn,  int PTLEVEL2SIZE,int PTLEVEL3SIZE);
int accessinglevel2(int N1, int N2, int N3, int va, uint32_t level_1_ppn, uint32_t level2mask, uint32_t level3mask, uint32_t maskoffset);
void addlevel2entry(int N3, int vpn, int PTLEVEL3SIZE);
int accessinglevel3(int N1, int N2, int N3, int va, uint32_t level_2_ppn, uint32_t level3mask, uint32_t maskoffset);
void addlevel3entry(int vpn);
//int totalaccess = 0;
