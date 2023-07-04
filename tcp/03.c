#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <string.h>

int main() {

    printf("tcp server start, pid =%d\r\n",getpid());

    // 1创建
    // 第三个参数一般给0
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    //  2绑定
    struct sockaddr_in address,client;
    address.sin_family = AF_INET;
    address.sin_port = htons(9501); // 9501
    address.sin_addr.s_addr = INADDR_ANY;


    int report = 1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&report, sizeof(report)) != 0){
        //
        printf("设置socket选项失败:\r\n");
    }

    int ret = bind(sockfd,(struct sockaddr*)(&address), sizeof(address));

    printf("bind ret = %d\r\n",ret);

    //   5
    ret = listen(sockfd,5);

    printf("listen ret = %d\r\n",ret);

    socklen_t client_len = sizeof(client);

    int connfd = accept(sockfd,(struct sockaddr*)&client,&client_len);

    printf("客户端连接上来，他的connfd=%d\r\n",connfd);

    char msg[1024] = {0};

    ret = recv(connfd,msg, sizeof(msg),0);

    printf("send %d bytes\r\n",ret);

    return 0;
}
