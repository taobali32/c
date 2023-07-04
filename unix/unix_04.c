#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
void sighandler(int signo){

    printf("signo=%d\r\n",signo);
}

#define UNIX "./unix_udp_test"

int main()
{
    int sockfd = socket(AF_LOCAL,SOCK_DGRAM,0);
    unlink(UNIX);

    struct sockaddr_un address,client;
    address.sun_family=AF_UNIX;
    strcpy(address.sun_path,UNIX);

    int ret = bind(sockfd,(struct sockaddr*)&address,sizeof(address));
    printf("bind ret=%d,errno=%d,error=%s\r\n",ret,errno,strerror(errno));


    char msg[1024]={0};

    socklen_t client_len = sizeof(client);
    ssize_t recvBytes = recvfrom(sockfd,msg,sizeof(msg),0,(struct sockaddr*)&client,&client_len);

    printf("recv %ld bytes,msg=%s\r\n",recvBytes,msg);


    ssize_t sendBytes = sendto(sockfd,"udp ok",6,0,(struct sockaddr*)&client,sizeof(client));

    printf("send %ld bytes\r\n",sendBytes);


    close(sockfd);



    return 0;
}
