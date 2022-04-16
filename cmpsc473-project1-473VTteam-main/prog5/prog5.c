#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
   if (argc !=2) {
        fprintf(stderr, "usage: cpu <string>\n");
        exit(1);
    }
    char *str = argv[1];
    int *p = malloc(sizeof(int));
    assert(p!= NULL);

    int i = 1;

    printf("PID: %d, Address pointed by p: %p\n",getpid(),p);

    while (i<5) {
        sleep(2);
        *p = *p + 1;
        printf("PID: %d, CPU: %s, *p: %d\n",getpid(),str,*p);
        i++;
    }
return 0;
}
