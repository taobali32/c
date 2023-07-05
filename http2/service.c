
#include "service.h"
#include "thread.h"
#include "epoll.h"
#include "reactor.h"
#include "connection.h"

#include <string.h>
#include <errno.h>
typedef struct connection connection;

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
    int ret = listen(server.sockfd,MAX_CONNECTIONS);

    if (ret == -1){
        perror("socket listen失败");
        exit(0);
    }

    printf("listen success ip=%s, port=%d\r\n", server.ip, server.port);
}

//  接收连接到
void Accept()
{
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int connfd = accept(server.sockfd,(struct sockaddr*)&client,&client_len);

    if(connfd>0){
        printf("客户端连接上来了，它的connfd=%d\r\n",connfd);
//        printf("recv %d,msg=%s,ip=%s,port=%d\r\n",recvBytes,msg,inet_ntoa(client.sin_addr),ntohs(client.sin_port));

        //eventAdd(&server,EPOLLIN,connfd);
        connection conn = {
                .sockfd=connfd,
                .port=ntohs(client.sin_port),
                //.ip=inet_ntoa(client.sin_addr),
                .recv_buffer=(char*)malloc(sizeof(char)*MAX_RECV_BUFFER),
                .send_buffer=(char*)malloc(sizeof(char)*MAX_SEND_BUFFER),
                .recv_last=0,
                .send_last=0,
                .recv_buffer_full=0,
                .send_buffer_full=0,
                .recv_max_bytes=MAX_SEND_BUFFER,
                .send_max_bytes=MAX_SEND_BUFFER
        };
        strcpy(conn.ip,inet_ntoa(client.sin_addr));

        reactor *cell = &server.cell[server.ix++%server.thread_num];
        addConnection(cell,conn);
    }


    //addConnection(cell,);
}

void *EventLoop()
{
    int epfd = epoll_create(MAX_CONNECTIONS);
    if(epfd==-1){
        printf("epoll_create failr\r\n");
        exit(0);
    }
    server.epfd=epfd;
    eventAdd(server.epfd,EPOLLIN,server.sockfd);
    struct epoll_event events[MAX_CONNECTIONS];

    while(server.run_flag){
        int ret = epoll_wait(epfd,events,MAX_CONNECTIONS,1);

        if(ret<0){
            if(errno==EINTR){
                continue;
            }
            break;
        }
        if(ret>0){
            for(int i=0;i<ret;i++){
                int fd = events[i].data.fd;
                //监听socket有客户端连接上来
                if(events[i].events&EPOLLIN &&fd==server.sockfd ){
                    Accept();
                }
            }
        }
    }

    printf("监听线程结束可以做一些清理工作\r\n");
//    notifyThread();

}
void Run()
{
    server.run_flag=1;
    //启动一个线程负责监听socket
    //启动threadNum线程负责连接socket
    createThread(EventLoop,NULL);

    server.cell = (reactor*)malloc(sizeof(reactor)*server.thread_num);

    for(int i =0;i<server.thread_num;i++){

        server.cell[i].ix=i;
        server.cell[i].current_client_num=0;
        server.cell[i].max_client_num=MAX_CONNECTIONS;
        server.cell[i].clients = (connection*)malloc(sizeof(connection)*MAX_CONNECTIONS);
        server.cell[i].clientsBuffer = (connection*)malloc(sizeof(connection)*MAX_CONNECTIONS);

        createThread(CellEventLoop,&server.cell[i].ix);
    }
}