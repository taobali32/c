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
#define UNIX_CLIENT "./unix_udp_client"

int main()
{
    int sockfd = socket(AF_LOCAL,SOCK_DGRAM,0);

    unlink(UNIX_CLIENT);
    
    struct sockaddr_un address,client;
    address.sun_family=AF_UNIX;
    strcpy(address.sun_path,UNIX);

    client.sun_family=AF_LOCAL;
    strcpy(client.sun_path,UNIX_CLIENT);

    int ret = bind(sockfd,(struct sockaddr*)&client,sizeof(client));
    printf("bind ret=%d,errno=%d,error=%s\r\n",ret,errno,strerror(errno));

    char msg[1024]={0};

    ssize_t sendBytes = sendto(sockfd,"china",5,0,(struct sockaddr*)&address,sizeof(address));

    printf("send %ld bytes\r\n",sendBytes);

    socklen_t len = sizeof(address);
    ssize_t recvBytes = recvfrom(sockfd,msg,sizeof(msg),0,(struct sockaddr*)&address,&len);
    printf("recv %ld bytes,msg=%s\r\n",recvBytes,msg);



    close(sockfd);



    return 0;
}
