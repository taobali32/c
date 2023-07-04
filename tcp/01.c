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


    while (1){
        socklen_t client_len = sizeof(client);

        int connfd = accept(sockfd,(struct sockaddr*)&client,&client_len);

        printf("客户端连接上来，他的connfd=%d\r\n",connfd);

        //  实现数据收发 recv
        char msg[1024] = {0};

        // 如果这里卡住不停的接收， 是没办法在接收新的连接的
        while (ret = recv(connfd,msg, sizeof(msg),0) <= 0){
            close(connfd);
            printf("客户端进程关闭了连接\r\n");
            break;
        }

        printf("recv %d bytes,msg=%s\r\n",ret,msg);

        // 清空数组
        memset(msg,0,sizeof (msg));

        // 发送数据 man 2 send

        char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello World";

        ret = send(connfd,resp, sizeof(resp),0);

        printf("send %d bytes\r\n",ret);
    }

    return 0;
}
