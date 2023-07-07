//
// Created by macos on 2023/7/5.
//

#ifndef C_CONNECTION_H
#define C_CONNECTION_H

#define MAX_RECV_BUFFER 1024
#define MAX_SEND_BUFFER 1024 * 1024 * 1024

#include <netinet/in.h>

#define INET_ADDRSTRLEN 16

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


//  客户端信息
typedef struct connection{

    http *data;

    int sockfd;  //  客户端fd
    int port;   //  客户端端口
    char ip[INET_ADDRSTRLEN];

    //  接收缓存区
    char *recv_buffer;
    //  发送缓冲区
    char *send_buffer;
    //  接收的最大字节数
    int recv_max_bytes;
    //  发送的最大字节数
    int send_max_bytes;

    //  目前接收到的字节数
    int recv_last;

    //  目前发送的字节数
    int send_last;

    int recv_buffer_full;
    int send_buffer_full;
} connection;

extern int recvData(connection *client);
extern int pushData(connection *client,char *data,int len);
extern int writeData(connection *client);
extern int hasMessage(connection *client);
extern int http_parse(connection *client);
extern char *splitStr(char *data,char *split,char *res);
extern int http_response(connection *client,char *data,int type);
extern int http_response_file(connection *client,char *data,int type,int len);
extern void popMessage(connection *client);
extern char *getHeader(connection *client,char *key);
extern char *getPost(connection *client,char *key);
extern char *getQuery(connection *client,char *key);

#endif //C_CONNECTION_H
