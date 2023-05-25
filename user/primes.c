#include <kernel/types.h>
#include <user/user.h>
#include "stddef.h"

void primes(int pptop){
    int ptoc[2], r, rc;
    int prime_num, num;
    if(read(pptop, &prime_num, sizeof(int)) == 0) exit(0);
    printf("prime %d\n", prime_num);

    pipe(ptoc);
    if ((r = fork()) == 0){
        close(ptoc[1]);
        primes(ptoc[0]);
        close(ptoc[0]);
    }
    else{
        close(ptoc[0]);
        while((rc = read(pptop, &num, sizeof(int)))){
            if(num % prime_num != 0) write(ptoc[1], &num, sizeof(int));
        }
        close(ptoc[1]);
        wait(NULL);
    }
}

int main(int argc, char *argv[]){
    int ptoc[2], r;
    pipe(ptoc);
    if((r = fork()) == 0){
        close(ptoc[1]);
        primes(ptoc[0]);
        close(ptoc[0]);
    }
    else{
        close(ptoc[0]);
        for(int i=2; i<35; i++){
            write(ptoc[1], &i, sizeof(int));
        }
        close(ptoc[1]);
        wait(NULL);
    }
    exit(0);
}