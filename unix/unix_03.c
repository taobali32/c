
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

    struct sockaddr_un address, client;

    address.sun_family = AF_UNIX;

    strcpy(address.sun_path,UNIX);

    int ret = connect(sockfd,(struct sockaddr*)&address, sizeof(address));

    printf("connfd ret = %d,errno=%d,error=%s\r\n",ret,errno, strerror(errno));

    char msg[1024] = {0};

    send(sockfd,"nihao",5,0);

    ret = recv(sockfd,msg, sizeof(msg),0);

    printf("client recv %d bytes,msg=%s\r\n",ret,msg);

    close(sockfd);

    return 0;
}

