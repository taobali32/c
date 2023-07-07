//
// Created by macos on 2023/7/4.
//

#ifndef CLESSON_SERVICE_H
#define CLESSON_SERVICE_H
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include "reactor.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "connection.h"
#include <string.h>

typedef struct Service{

    int sockfd;
    int port;
    char ip[INET_ADDRSTRLEN];
    int backlog;
    int thread_num;
    int ix;
    int epfd;

    int run_flag;
    reactor *cell;
    void (*InitSocket)(void);
    void (*Bind)(void);
    void (*Listen)(void);
    void (*Run)(void);
    void (*Stop)(void);

    void (*onRequest)(connection *client);

} Service;


extern Service server;
extern void initSocket();
extern void Bind();
extern void Listen();
extern void Run();
extern void Stop();
extern void onRequest(connection *client);

#endif //CLESSON_SERVICE_H

