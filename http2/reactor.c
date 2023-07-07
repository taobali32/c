//
// Created by macos on 2023/7/5.
//
#include "service.h"
#include "epoll.h"
#include "connection.h"

#include <pthread.h>

#include <string.h>
#include "thread.h"

// 加static 变成局部 变量
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void addConnection(reactor *cell,connection client)
{
    int i;
    // 线程锁着，
    pthread_mutex_lock(&mutex);
    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        //  如果为0，表示该位置的连接缓冲区为空，可以将新的连接添加到这个位置。
        if(cell->clientsBuffer[i].sockfd==0){
            break;
        }
    }

    cell->clientsBuffer[i] = client;
    //  增加当前连接数的计数。
    cell->current_client_num++;

    pthread_mutex_unlock(&mutex);

}

connection *findClient(reactor *cell,int fd){
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        if(cell->clients[i].sockfd == fd && cell->clients[i].sockfd!=0){
            return &cell->clients[i];
        }
    }

    return NULL;
}


void removeClient(reactor *cell,connection *client){
    //
    client->recv_last=0;
    client->send_last=0;
    client->send_max_bytes=0;
    client->recv_max_bytes=0;
    client->recv_buffer_full=0;
    // client->ip={0};
    memset(client->ip,0, sizeof(client->ip));
    client->port=0;
    memset(client->recv_buffer,0, sizeof(client->recv_buffer));
    memset(client->send_buffer,0, sizeof(client->send_buffer));
    free(client->data);
    eventDel(cell->epfd,EPOLLIN,client->sockfd);

    int x = close(client->sockfd);

    //
    printf("x=%d\r\n",x);

    printf("客户端关闭了连接%d\r\n",client->sockfd);
    client->sockfd=0;
}

int set_file_nonblock(int fd){
    //把文件设置为非阻塞模式  非阻塞的文件描述符称为非阻塞IO
    int option = fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL,option|O_NONBLOCK);
    return 1;
}

//  子线程
void *CellEventLoop(void *arg){

    printf("走到CellEventLoop\r\n");

    int ix = *(int *)arg;
    //  epoll创建
    int epfd = epoll_create(MAX_CONNECTIONS);

    if (epfd == -1){
        printf("epoll create fail\r\n");
        exit(0);
    }

    reactor *cell = &server.cell[ix];
    cell->epfd = epfd;

    printf("走到这里了吗?");

    struct epoll_event events[MAX_CONNECTIONS];
    while (server.run_flag){

        pthread_mutex_lock(&mutex);
        //  当前连接>0,收到客户端连接后++

        // 上面 连接成功后 初始化 这里 自增过 cell->current_client_num
        if (cell->current_client_num > 0){
            for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                //  addConnection 添加的这里执行
                if (cell->clientsBuffer[i].sockfd > 0){

                    for (int j = 0; j < MAX_CONNECTIONS; ++j) {

                        // 内层的循环遍历clients数组，寻找一个空闲的位置（sockfd == 0）。
                        // 一旦找到空闲位置，就将clientsBuffer中的连接复制到该位置，
                        // 并进行相应的处理（例如将连接添加到事件循环中）。

                        if (cell->clients[j].sockfd == 0){
                            cell->clients[j] = cell->clientsBuffer[i];

                            // 设置文件非阻塞
                            set_file_nonblock(cell->clients[j].sockfd);
                            eventAdd(cell->epfd,EPOLLIN,cell->clients[j].sockfd);
                            break;
                        }
                    }

                    // buffer连接移动到clients里面后重置
                    cell->clientsBuffer[i].sockfd = 0;
                }
            }
            // 把有效连接添加后重置为0 退出while
            cell->current_client_num = 0;
        }

        pthread_mutex_unlock(&mutex);

        int ret = epoll_wait(cell->epfd,events,MAX_CONNECTIONS,1);

        if (ret < 0){
            if (errno == EINTR){
                continue;
            }
            break;
        }

        if (ret > 0){
            for (int i = 0; i < ret; ++i) {
                int fd = events[i].data.fd;

                //  EPOLLIN
                if (events[i].events & EPOLLIN){

                    // 继续封装数据发送!
                    connection *client = findClient(cell,fd);

                    /**
                     * 收到数据 处理，并且要清理下缓存区的内容！， 现在没传递参数，数据就是ip,请求方式，请求头那些.
                     */
                    printf("recv msg=%s\r\n",client->recv_buffer);


                    //  不然等下缓冲区就满了, 先准备一块缓冲区，等下放数据
                    memset(client->recv_buffer,0, sizeof(client->recv_buffer));
                    client->recv_last = 0;

                    //
                    if (recvData(client) == -1){
                        //  关闭
                        removeClient(cell,client);
                    } else{

                        if (hasMessage(client) != -1){
                            //  已经接收一条完整的消息
                            http_parse(client);

                            //执行onRequest事件 业务逻辑一般在这函数处理
                            server.onRequest(client);

                            popMessage(client);
                        }

                        int writeRet = writeData(client);

                        printf("writeRet=%d\r\n",writeRet);
                        if (writeRet == -1) {
                            removeClient(cell, client);
                        } else if (writeRet > 0) {
                            eventSet(cell->epfd, EPOLLOUT | EPOLLIN, client->sockfd);
                        } else if (writeRet == 0){
                            printf("走到了writeRet=%d\r\n",writeRet);

                            removeClient(cell, client);
                        }
                    }
                }

                if (events[i].events & EPOLLOUT){
                    //
                    connection *client = findClient(cell,fd);

                    int writeRet = writeData(client);

                    //发送了一半，还没发送完
                    if (writeData(client) == -1){
                        removeClient(cell,client);
                    } else if (writeRet > 0){
                        eventSet(client->sockfd,EPOLLIN,client->sockfd);
                    }else if (writeRet == 0){
                        removeClient(cell, client);
                    }
                }
            }
        }
    }

    printf("连接线程结束可以做一些清理工作\r\n");
    close(cell->epfd);
    for (int i = 0; i < MAX_CONNECTIONS; ++i){

        if (cell->clientsBuffer[i].sockfd > 0) {

            close(cell->clientsBuffer[i].sockfd);
        }
        if (cell->clients[i].sockfd > 0) {

            close(cell->clients[i].sockfd);
        }
    }
    free(cell->clients);
    free(cell->clientsBuffer);
    notifyThread();
}