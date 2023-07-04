#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
    if(access("./fifox",F_OK)){
        int ret = mkfifo("./fifox",0666);
    }

    int fd = open("./fifox",O_WRONLY);

    char msg[128]={0};
    while(1){

        fgets(msg,sizeof(msg),stdin);

        if(strncmp("exit",msg,4)==0){

            break;
        }
        write(fd,msg,strlen(msg));

        memset(msg,0,sizeof(msg));
    }
    close(fd);//当这里关闭时，对端fd[0]还在读的话，就会返回0，表示fd[1]已经关闭了
    exit(0);
}
