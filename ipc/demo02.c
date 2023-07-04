#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>

int main(){

    if (access("./fifo",F_OK)){
        int ret = mkfifo("./fifo",0666);
    }

    int fd = open("./fifo",O_RDONLY);

    char msg[128] = {0};

    while (1){
        memset(msg,0, sizeof(msg));

        int bytes = read(fd,msg, sizeof(msg));

        if(bytes == 0){
            break;
        }

        printf("收到%d bytes,数据:%s\r\n",bytes,msg);
    }

    close(fd);

    return 0;
}