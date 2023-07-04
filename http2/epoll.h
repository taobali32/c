//
// Created by macos on 2023/7/4.
//

#ifndef C_EPOLL_H
#define C_EPOLL_H

#include "service.h"

#define MAX_EVENTS 1024


extern void eventDel(Service *service,int event,int sockfd);
extern void eventAdd(Service *service,int event,int sockfd);

#endif //C_EPOLL_H
