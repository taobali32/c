#include <stdio.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <signal.h>


void singHandler(int signo){
    printf("signo=%d\r\n",signo);
}

int main() {

    signal(SIGPIPE,singHandler);

    int fds[2];
    //  SOCK_STREAM  TCP
    //  SOCK_DGRAM   UDP
//    int ret = socketpair(AF_UNIX,SOCK_STREAM,0,fds);

    //  SOCK_NONBLOCK 不阻塞
    int ret = socketpair(AF_UNIX,SOCK_DGRAM|SOCK_NONBLOCK,0,fds);
    //
    printf("rest = %d,error=%d,error=%s\r\n",ret,errno, strerror(errno));

    int pid = fork();

    if (pid == 0) {

        while (1){
            char msg[1024] = {0};

            int ret = recv(fds[0], msg, sizeof(msg), 0);
            //
            if (ret <= 0){
                printf("对端关闭\r\n");
                break;
            }

            printf("child recv %d bytes,msg= %s\r\n", ret, msg);
        }

        exit(0);
    }

    ret = write(fds[1],"china",5);
    write(fds[1],"hello",5);

    close(fds[1]);
    close(fds[0]);

    printf("parent send %d bytes\r\n",ret);

    wait(NULL);
    return 0;
}

