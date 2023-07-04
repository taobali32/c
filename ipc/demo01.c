#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>

int main(){

    if (access("./fifo",F_OK)){
        int ret = mkfifo("./fifo",0666);
    }

    int fd2 = open("./fifo",O_WRONLY);

    char msg[128] = {0};
    while (1){
        fgets(msg,sizeof (msg),stdin);

        if (strncmp("exit",msg,4) == 0){
            break;
        }

        write(fd2,msg, strlen(msg));

        memset(msg,0, sizeof(msg));
    }

    close(fd2);

    return 0;
}