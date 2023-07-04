#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <string.h>
#include <arpa/inet.h>

int main() {

    printf("udp client pid =%d\r\n",getpid());

    // 1创建
    // 第三个参数一般给0
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);

    //  2绑定
    struct sockaddr_in address,client;
    address.sin_family = AF_INET;
    address.sin_port = htons(9501); // 9501
    address.sin_addr.s_addr = INADDR_ANY;

    ssize_t  ret = sendto(sockfd,"hello",5,0,(struct sockaddr*)&address, sizeof(address));

    printf("sendto ret = %zd\r\n",ret);


    // 接收

    char msg[1024] = {0};
    socklen_t client_len = sizeof(client);

    ssize_t recvBytes = recvfrom(sockfd,msg, sizeof(msg),0,(struct sockaddr*)&client,&client_len);
    printf("recv %zd,msg = %s,ip=%s,port=%d\r\n",recvBytes,msg,inet_ntoa(client.sin_addr), ntohs(client.sin_port));


    return 0;
}
