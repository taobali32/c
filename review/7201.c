#include <stdio.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#include <signal.h>

int main() {

    signal(SIGUSR1,SIG_IGN);

    //
    printf("tcp server start,pid=%d\r\n",getpid());

    int sockfd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in address,client;

    address.sin_family = AF_INET;
    address.sin_port = htons(9501);
    address.sin_addr.s_addr = inet_addr("0.0.0.0");

    int report = 1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&report, sizeof(report)) != 0){
        printf("设置socket选项失败\r\n");
    }

    //
    int ret = bind(sockfd,(struct sockaddr*)&address, sizeof(address));

    printf("bind ret=%d,errno=%d,error=%s\r\n",ret,errno, strerror(errno));

    ret = listen(sockfd,5);

    // 创建一个epoll文件描述符，它内核关联的内核事件表（红黑树实现）
    int epfd = epoll_create(1024);
    //
    if (epfd == -1){
        printf("epoll create fail\r\n");
        exit(0);
    }

    struct epoll_event event;

    event.events = EPOLLIN;
    event.data.fd = sockfd;

    // 把sockfd监听socket添加到epoll内核事件表中,文件socketfd要关注的事件是可读事件
    //  监听
    //  可读事件发送情况
    //  客户端连接上来时
    //  客户端断开时
    //  异常
    //  客户端发送数据上来，并且达到内核的相关设置【接收水位标志】
    //

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD,sockfd,&event);

    struct epoll_event events[1024];

    while (1){
        //  -1 阻塞
        // 返回就绪的
        ret = epoll_wait (epfd,events,1024,-1);

        if (ret < 0){

            if (errno == EINTR){
                continue;
            }
            // 没数据也不能这里退出进程
            printf("ret=%d,errno=%d,error=%s\r\n",ret,errno, strerror(errno));
            break;
        }

        if (ret > 0){
            for (int i = 0; i < ret; ++i) {
                int fd = events[i].data.fd;

                //  监听socket有客户端连接上来
                if (events[i].events && EPOLLIN){

                    //  监听socket有客户端连接上来了
                    if (fd == sockfd ){
                        printf("listen ret = %d\r\n",ret);

                        socklen_t client_len = sizeof(client);
                        int connfd = accept(sockfd,( struct sockaddr*)&client,&client_len);
                        printf("客户端连接上来了,他的connfd=%d\r\n",connfd);

                        struct epoll_event event;
                        event.events = EPOLLIN;
                        event.data.fd = connfd;

                        ret = epoll_ctl(epfd, EPOLL_CTL_ADD,connfd,&event);
                    } else{
                        char msg[1024] = {0};

                        ret = recv(fd,msg, sizeof(msg),0);

                        printf("receive %d bytes,msg=%s\r\n",ret,msg);
                        if (ret > 0){
                            char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello World";
                            ret = send(fd,resp, sizeof(resp),0);
                            printf("send %d bytes\r\n",ret);
                        } else if (ret <= 0){
                            //  如果对端关闭了，应该移除掉
                            close(fd);
                            ret = epoll_ctl(epfd, EPOLL_CTL_DEL,fd,NULL);

                        }

                    }

                }
            }
        }
    }

    return 0;
}
