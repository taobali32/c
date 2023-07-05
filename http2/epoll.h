//
// Created by macos on 2023/7/4.
//

#ifndef C_EPOLL_H
#define C_EPOLL_H

#include "service.h"

#define MAX_CONNECTIONS 1024

extern void eventAdd(int epfd,int event,int sockfd);
extern void eventDel(int epfd,int event,int sockfd);

extern void eventSet(int epfd,int event,int sockfd);

#endif //C_EPOLL_H
