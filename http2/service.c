
#include "service.h"
#include "thread.h"
#include "epoll.h"
#include "reactor.h"
#include "connection.h"

#include <string.h>
#include <errno.h>

typedef struct connection connection;

void initSocket(){
    //  SOCK_NONBLOCK是一个套接字选项，用于设置套接字为非阻塞模式。
    int sockfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);

    if (sockfd < 0){
        printf("socket创建失败\r\n");
        exit(0);
    }

    printf("server.sockfd=%d\r\n",sockfd);
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
    //  int listen(int sockfd, int backlog);
    //  backlog是一个整数，指定等待连接队列的最大长度。当有新的连接请求到达时，如果等待连接队列已满，新的连接请求将被拒绝。
    int ret = listen(server.sockfd,MAX_CONNECTIONS);

    if (ret == -1){
        perror("socket listen失败");
        exit(0);
    }

    printf("listen success ip=%s, port=%d\r\n", server.ip, server.port);
}

//  接收连接
void Accept()
{
    //  struct sockaddr_in是一个用于存储IPv4地址信息的结构体。
    //  它通常用于网络编程中，用于表示一个客户端或服务器的地址信息。
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);

    int connfd = accept(server.sockfd,(struct sockaddr*)&client,&client_len);

    printf("客户端connfd=%d\r\n",connfd);

    if(connfd>0){
        printf("客户端连接上来了，它的connfd=%d\r\n",connfd);

        //  连接信息存储
        connection conn = {
                .sockfd=connfd,
                .port=ntohs(client.sin_port),
                .recv_buffer=(char*)malloc(sizeof(char)*MAX_RECV_BUFFER),
                .send_buffer=(char*)malloc(sizeof(char)*MAX_SEND_BUFFER),
                .recv_last=0,
                .send_last=0,
                .recv_buffer_full=0,
                .send_buffer_full=0,
                .recv_max_bytes=MAX_SEND_BUFFER,
                .send_max_bytes=MAX_SEND_BUFFER
        };

        //  因为ip是数组. 拷贝过来
        strcpy(conn.ip,inet_ntoa(client.sin_addr));

        // 找到某个线程处理
        reactor *cell = &server.cell[server.ix++%server.thread_num];
        addConnection(cell,conn);
    }
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

    //
    while(server.run_flag){
        int ret = epoll_wait(epfd,events,MAX_CONNECTIONS,1);

        if(ret<0){
            //  当一个进程正在执行一个系统调用（如读取文件、写入文件、等待信号等）时，
            //  如果收到一个信号，系统调用可能会被中断。此时，系统会将errno设置为EINTR，
            //  以表示系统调用被中断。程序可以通过检查errno的值来判断系统调用是否被中断，
            //  然后采取相应的处理措施。
            if(errno==EINTR){
                continue;
            }
            break;
        }
        if(ret>0){
            for(int i=0;i<ret;i++){
                int fd = events[i].data.fd; //  获取当前事件对应的文件描述符。
                //  判断当前事件是否是可读事件，并且对应的文件描述符是否是监听socket的文件描述符。
                if(events[i].events&EPOLLIN &&fd==server.sockfd ){
                    Accept();
                }
            }
        }
    }

    printf("监听线程结束可以做一些清理工作\r\n");
    notifyThread();

}

void Run()
{
    server.run_flag=1;
    //启动threadNum线程负责连接socket
    createThread(EventLoop,NULL);

    // 线程数组，后续轮训某个线程处理数据
    server.cell = (reactor*)malloc(sizeof(reactor)*server.thread_num);

    for(int i =0;i<server.thread_num;i++){

        server.cell[i].ix=i;
        server.cell[i].run_flag = 1;
        server.cell[i].current_client_num=0;
        server.cell[i].max_client_num=MAX_CONNECTIONS;
        server.cell[i].clients = (connection*)malloc(sizeof(connection)*MAX_CONNECTIONS);
        server.cell[i].clientsBuffer = (connection*)malloc(sizeof(connection)*MAX_CONNECTIONS);

        createThread(CellEventLoop,&server.cell[i].ix);
    }
}

void Stop(){

    //主线程调用并用阻塞当前线程
    stopListenThread(&server);
    for(int i =0;i<server.thread_num;i++){
        stopConnectThread(&server.cell[i]);
    }
    free(server.cell);
    close(server.epfd);
    close(server.sockfd);
}