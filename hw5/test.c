
#include <stdio.h>


int main(int argc, char * argv[]) {
    int i;
    for (i = 0; i < argc; i++) {
        sleep (2*i+1);
        printf("Arg %d is %s\n",i+1, argv[i]);
    }

}