//
// Created by macos on 2023/7/4.
//

#include "service.h"


void eventAdd(Service *service,int event,int sockfd){
    struct epoll_event events;
    events.events = event; //;
    events.data.fd = sockfd;

    int ret = epoll_ctl(service->epfd,EPOLL_CTL_ADD,sockfd,&events);

    //  失败
    if (ret != 0){
        perror("epoll_ctl fail");
    }
}


void eventDel(Service *service,int event,int sockfd){
    struct epoll_event events;
    events.events = event; //;
    events.data.fd = sockfd;

    //
    int ret = epoll_ctl(service->epfd,EPOLL_CTL_DEL,sockfd,&events);

    //  失败
    if (ret != 0){
        perror("epoll_ctl fail");
    }
}
