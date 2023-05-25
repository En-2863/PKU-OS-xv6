#include <kernel/types.h>
#include <kernel/stat.h>
#include <user/user.h>
#include <kernel/param.h>
#include "stddef.h"


int main(int argc, char** argv){
    char buf[512], *p;
    char *new_argv[MAXARG];
    int r;
    p = buf;
    while(read(0, p, 1)){
        if(p >= buf + 512){
            printf("command size out of range.\n");
        }
        if(*p == '\n'){
            *p = '\0';
            if (argc + 1 >= MAXARG){
                printf("too many args.\n");
                exit(1);
            }
            memmove(new_argv, &argv[1], sizeof(char*)*(argc - 1));
            new_argv[argc - 1] = (char*)malloc((strlen(buf) + 1)*sizeof(char));
            memmove(new_argv[argc - 1], buf, strlen(buf) + 1);
            new_argv[argc] = NULL;
            if((r = fork()) == 0){
                exec(new_argv[0], new_argv);
            }
            else{
                wait(NULL);
                p = buf;
                free(new_argv[argc - 1]);
            }
        }
        else p++;
    }
    exit(0);
}