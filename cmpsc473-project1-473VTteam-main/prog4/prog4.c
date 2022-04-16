#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "check.h"

int x[5] = {1,2,3,4,5};

void allocate()
{
    int i;
    int *p;
    for(i =1 ; i<1000000 ; i++)
    {
      p = malloc(500 * sizeof(int));
      if(func(i)) { free (p);}
    }
}

void allocate1()
{
  int i;
  int *p;
  for (i=1 ; i<10000 ; i++)
  {
    p = malloc(1000 * sizeof(int));
    if(i & 1)
      free (p);
  }
}

void allocate2()
{
  int i;
  int *p;
  for (i=1 ; i<300000 ; i++)
  {
    p = malloc(10000 * sizeof(int));
    free (p);
  }
}


int main(int argc, char const *argv[]) {
  int i;
  int *p;
  struct rusage usage;
  struct timeval start, end, start2, end2;
  printf("Executing the code ......\n");
  getrusage(RUSAGE_SELF, &usage); 
  start = usage.ru_utime; 
  start2 = usage.ru_stime;
  
  allocate();

  for (i=0 ; i<10000 ; i++)
  {
    p = malloc(1000 * sizeof(int));
    free (p);
  }

  getrusage(RUSAGE_SELF, &usage);
  end = usage.ru_utime;
  end2 = usage.ru_stime;

  printf("User cpu time: Started at: %ld.%lds\n", start.tv_sec, start.tv_usec);
  printf("User cpu time: Ended at: %ld.%lds\n", end.tv_sec, end.tv_usec);
  printf("System cpu time: Started at: %ld.%lds\n", start2.tv_sec, start2.tv_usec);
  printf("System cpu time: Ended at: %ld.%lds\n", end2.tv_sec, end2.tv_usec);
  printf("Max resident size: %ld\n", usage.ru_maxrss);
  printf("Signals: Started at: %ld\n", usage.ru_nsignals);
  printf("Voluntary context switches: %ld\n", usage.ru_nvcsw);
  printf("Involuntary context switches: %ld\n", usage.ru_nivcsw);
  printf("Program execution successfull\n");
  return 0;
}

