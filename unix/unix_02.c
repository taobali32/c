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

#include <sys/socket.h>
#include <sys/un.h>


#define UNIX "./unix_tests"

int main() {

    int sockfd = socket(AF_LOCAL,SOCK_STREAM,0);
    unlink(UNIX);

    struct sockaddr_un address, client;

    address.sun_family = AF_UNIX;


    strcpy(address.sun_path,UNIX);

    int ret = bind(sockfd,(struct sockaddr*)&address, sizeof(address));
    printf("bind ret = %d,errno=%d,error=%s\r\n",ret,errno, strerror(errno));

    ret = listen(sockfd,128);
    printf("listen ret = %d,errno=%d,error=%s\r\n",ret,errno, strerror(errno));

    socklen_t len = sizeof(client);
    int connfd = accept(sockfd,(struct sockaddr*)&client,&len);

    printf("connfd=%d\r\n",connfd);

    char msg[1024] = {0};
    ret = recv(connfd,msg, sizeof(msg),0);

    printf("recv %d bytes,msg = %s\r\n",ret,msg);

    send(connfd,"china",5,0);

    close(connfd);
    close(sockfd);

    return 0;
}

