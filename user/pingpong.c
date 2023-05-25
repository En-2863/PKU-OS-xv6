#include <kernel/types.h>
#include <user/user.h>
#include "stddef.h"

int main(int argc, char* argv[]){
    int ptoc[2], ctop[2], r;
    char buf[10];
    pipe(ptoc);
    pipe(ctop);
    if ((r = fork()) == 0){
        read(ptoc[0], buf, 4);
        printf("%d: received %s\n", getpid(), buf);
        write(ctop[1], "pong", strlen("pong"));
    }
    else{
        write(ptoc[1], "ping", strlen("ping"));
        wait(NULL);
        read(ctop[0], buf, 4);
        printf("%d: received %s\n", getpid(), buf);
    }
    exit(0);
}