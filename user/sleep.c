#include "kernel/types.h"
#include "user/user.h"

extern uint64 sys_sleep(void);
int main(int argc, char* argv[]){
    if (argc != 2){
        write(2, "Usage: sleep time\n", strlen("Usage: sleep time\n"));
        exit(1);
    }
    uint tick_dur = (int)*argv[1];
    sleep(tick_dur);
    return 0;
}