
#include "service.h"
#include "thread.h"
#include "epoll.h"
#include "reactor.h"

void initSocket(){
    int sockfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);

    if (sockfd < 0){
        printf("socket创建失败\r\n");
        exit(0);
    }

    server.sockfd = sockfd;
}

void Bind(){
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(server.port);
    address.sin_addr.s_addr = inet_addr(server.ip);

    int report = 1;

    /**
     * setsockopt()函数用于设置Socket选项。
     * setsockopt()函数被用来设置server.sockfd套接字的SO_REUSEPORT选项。
     * SO_REUSEPORT选项允许多个套接字绑定到同一个端口上，
     * 这样就可以实现负载均衡和高可用性。
     */
    if (setsockopt(server.sockfd,SOL_SOCKET,SO_REUSEPORT,&report, sizeof(report)) != 0){
        printf("设置socket选项失败\r\n");
    }

    int ret = bind(server.sockfd,(struct sockaddr*)&address, sizeof(address));

    if (ret == -1){
        printf("socket bind 失败\r\n");
    }
}

void Listen(){
    int ret = listen(server.sockfd,MAX_EVENTS);

    if (ret == -1){
        perror("socket listen失败");
        exit(0);
    }

    printf("listen success ip=%s, port=%d\r\n", server.ip, server.port);
}

void Accept(){
    struct sockaddr_in client;

    socklen_t client_len = sizeof(client);
    int connfd = accept(server.sockfd,(struct sockaddr*)&client,&client_len);
    printf("客户端连接上来了，他的connfd=%d\r\n",connfd);

    eventAdd(&server,EPOLLIN,connfd);
}


void *EventLoop(){
    //  epoll创建
    int epfd = epoll_create(MAX_EVENTS);

    if (epfd == -1){
        printf("epoll create fail\r\n");
        exit(0);
    }

    server.epfd = epfd;
    eventAdd(&server,EPOLLIN,server.sockfd);

    struct sockaddr_in address;

    struct epoll_event events[MAX_EVENTS];

    while (1){
        //
        int ret = epoll_wait(epfd,events,MAX_EVENTS,1);

        if (ret < 0){
            if (errno == EINTR){
                continue;
            }
            break;
        }

        if (ret > 0){
            for (int i = 0; i < ret; ++i) {
                int fd = events[i].data.fd;

                if (events[i].events && EPOLLIN){
                    if (fd == server.sockfd){
                        Accept();
                    } else{
                        char msg[1024] = {0};
                        ret = recv(fd,msg, sizeof(msg),0);

                        printf("receive %d types,msg=%s\r\n",ret,msg);

                        printf("receive %d bytes,msg=%s\r\n",ret,msg);
                        if (ret > 0){
                            char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello World";
                            ret = send(fd,resp, sizeof(resp),0);
                            printf("send %d bytes\r\n",ret);
                        } else if (ret <= 0){
                            //  如果对端关闭了，应该移除掉
                            close(fd);
                            eventDel(&server,EPOLLIN,fd);
                        }
                    }
                }
            }
        }
    }
}

void Run(){
    //  启动一个线程负责socket
    //  启动线程负责连接socket
    createThread(EventLoop,NULL);
}