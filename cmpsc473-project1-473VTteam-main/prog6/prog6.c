#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr,"usage: cpu <string>\n");
        exit(1);
    }

    char *name = argv[1];
    int num1 = atoi(argv[2]);
    int num2 = atoi(argv[3]);

    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }

    else if (rc == 0) {
        int sum = num1 + num2;
        printf("Sum of the two numbers is %d\n",sum);
    }

    else {
        printf("My name is %s\n",name);
    }
return 0;
}
