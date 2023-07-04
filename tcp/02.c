#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/tcp.h>

int main() {

    printf("tcp client start, pid =%d\r\n",getpid());

    // 1创建
    // 第三个参数一般给0
    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    //  2绑定
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = 0x1d25; // 9501
    address.sin_addr.s_addr = INADDR_ANY;

    int ret = connect(sockfd,(struct sockaddr*)&address, sizeof(address));

    printf("connect ret=%d\r\n",ret);

    ret = send(sockfd,"china",5,0);

    printf("send %d bytes\r\n",ret);

}
