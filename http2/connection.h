//
// Created by Administrator on 2021/8/9 0009.
//

#ifndef CLESSON_CONNECTION_H
#define CLESSON_CONNECTION_H

#include <netinet/in.h>

#define MAX_RECV_BUFFER 1024*1024*10
#define MAX_SEND_BUFFER 1024*1024*10

typedef struct kv{
    char *key;
    char *val;
}kv;
typedef struct http{
    kv *header;
    kv *post;
    kv *get;

    int body_len;
    int header_len;

    char *method;
    char *uri;
    char *version;
}http;
typedef struct connection{

    http *data;

    int sockfd;
    int port;
    char ip[INET_ADDRSTRLEN];

    char *recv_buffer;//接收缓冲区
    char *send_buffer;//发送缓冲区
    int recv_max_bytes;//接收的最大字节数
    int send_max_bytes;//发送的最大字节数
    int recv_last;//目前已经接收的字节数
    int send_last;//目前已经发送的字节数

    int recv_buffer_full;
    int send_buffer_full;

}connection;

extern int recvData(connection *client);
extern int pushData(connection *client,char *data,int len);
extern int writeData(connection *client);
extern int hasMessage(connection *client);
extern int http_parse(connection *client);
extern char *splitStr(char *data,char *split,char *res);
extern char *getHeader(connection *client,char *key);
extern char *getPost(connection *client,char *key);
extern char *getQuery(connection *client,char *key);
extern int http_response(connection *client,char *data,int type);
extern int http_response_file(connection *client,char *data,int type,int len);
extern void popMessage(connection *client);
#endif //CLESSON_CONNECTION_H
