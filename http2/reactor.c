//
// Created by macos on 2023/7/5.
//
#include "service.h"
#include "epoll.h"
#include "connection.h"

#include <pthread.h>

#include <string.h>
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void addConnection(reactor *cell,connection client)
{

    int i;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < MAX_CONNECTIONS; ++i) {
        if(cell->clientsBuffer[i].sockfd==0){
            break;
        }
    }

    cell->clientsBuffer[i] = client;
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
    client->recv_last = 0;
    client->send_last = 0;
    client->send_max_bytes = 0;
    client->recv_max_bytes = 0;
    client->recv_buffer_full = 0;
    memset(client->ip,0, sizeof(client->ip));
    client->port = 0;

    memset(client->recv_buffer, 0,sizeof(client->recv_buffer));
    memset(client->send_buffer, 0,sizeof(client->send_buffer));

    eventDel(cell->epfd,EPOLLIN,client->sockfd);

    close(client->sockfd);

    printf("客户端关闭了连接:%d\r\n",client->sockfd);

    client->sockfd = 0;

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
    while (1){

        pthread_mutex_lock(&mutex);
        //  当前连接>0,收到客户端连接后++
        if (cell->current_client_num > 0){
            for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                // 肯定>0
                if (cell->clientsBuffer[i].sockfd > 0){

                    //多次的目的是用于将clientsBuffer中的连接移到clients数组中。
                    /**
                     * 根据代码片段来看，循环中的逻辑是将clientsBuffer中当前存在的连接移动到clients数组中。
                     * clientsBuffer数组可能是用来缓存新连接的数组，而clients数组可能是用来存储正在处理的连接的数组。
                        这样的设计可能是为了避免在处理连接的过程中修改clientsBuffer数组导致不一致的情况。
                        通过循环多次将clientsBuffer中的连接移动到clients数组中，确保clientsBuffer中的所有连接都被转移到clients数组中后，再进行后续的处理。
                        总结来说，循环多次的目的是将缓存连接的数组中的连接移动到正在处理的连接的数组中，确保所有的连接都得到处理。
                     */
                    for (int j = 0; j < MAX_CONNECTIONS; ++j) {
                        if (cell->clients[j].sockfd == 0){
                            cell->clients[j] = cell->clientsBuffer[i];
                            //
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

                if (events[i].events & EPOLLIN){

                    // 继续封装数据发送!
                    connection *client = findClient(cell,fd);

                    /**
                     * 收到数据 处理，并且要清理下缓存区的内容！
                     */
                    printf("recv msg=%s\r\n",client->recv_buffer);

                    //  不然等下缓冲区就满了
                    memset(client->recv_buffer,0, sizeof(client->recv_buffer));
                    client->recv_last = 0;

                    if (recvData(client) == -1){
                        //  关闭
                        removeClient(cell,client);
                    } else{
                        //
                        char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello World";
                        pushData(client,resp, strlen(resp));

                        int writeRet = writeData(client);

                        //发送了一半，还没发送完
                        if (writeData(client) == -1){
                            removeClient(cell,client);
                        } else if (writeRet > 0){
                            //
                            eventSet(client->sockfd,EPOLLOUT|EPOLLIN,client->sockfd);
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
                    }
                }
            }

        }
    }
}

