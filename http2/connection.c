//
// Created by macos on 2023/7/5.
//
#include "connection.h"

#include <string.h>
#include <stdio.h>
#include "service.h"
//  接收
int recvData(connection *client)
{
    if(client->sockfd == 0){
        return -1;
    }

    // 可以接收的最大字节 - 最后已接收的字节
    //  目前缓冲区剩下长度
    int remain_len = client->recv_max_bytes - client->recv_last;

    if(remain_len > 0){
        //  从指定位置接收数据
        char *data = client->recv_buffer+client->recv_last;

        int recvBytes =  recv(client->sockfd,data, remain_len,0);

        if(recvBytes == 0){
            //  对端已经关闭
            return -1;
        }

        if(recvBytes < 0){
            printf("recvDate recv fail %s\r\n", strerror(errno));
            return -1;
        }

        //接收字节累加
        client->recv_last += recvBytes;
        client->recv_buffer[client->recv_last] = 0;

        return recvBytes;
    } else{
        client->recv_buffer_full++;
    }

    return 0;
    //
//    char msg[1024] = {0};
//    ret = recv(fd,msg, sizeof(msg),0);

//    printf("receive %d bytes,msg=%s\r\n",ret,msg);
}
int pushData(connection *client,char *data,int len){
    if(client->sockfd == 0){
        return -1;
    }

    // 目前放在缓存中的字节
    if (client->send_last + len < client->send_max_bytes){
        memcpy(client->send_buffer + client->send_last,data,len);

        client->send_last+=len;

        if (client->send_last >= client->send_max_bytes){
            client->send_buffer_full++;
        }

    } else{
        client->send_buffer_full++;
    }

    return 0;

}

int writeData(connection *client){
    if(client->sockfd==0){
        return -1;
    }

    if(client->send_last>0){

        int sendBytes = send(client->sockfd,client->send_buffer,client->send_last,0);

        if(sendBytes==0){
            printf("writeData fail %s\r\n",strerror(errno));
            return -1;
        }
        else if(sendBytes<0){
            printf("writeData fail %s\r\n",strerror(errno));
            return -1;
        }else if (sendBytes==client->send_last){

            //一次性发送完
            memset(client->send_buffer,0, sizeof(client->send_buffer));
            client->send_last=0;
            client->send_buffer_full=0;
        }else{

            //发送一半的情况下
            client->send_last-=sendBytes;
            memcpy(client->send_buffer,client->send_buffer+sendBytes,client->send_last);
            return sendBytes;
        }

    }

    return 0;

//    char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nHello World";

}