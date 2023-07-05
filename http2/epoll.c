//
// Created by macos on 2023/7/4.
//

#include "service.h"


void eventAdd(int epfd,int event,int sockfd)
{
    struct epoll_event events;
    events.events=event;
    events.data.fd=sockfd;

    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&events);
    if(ret!=0){
        perror("epoll_ctl EPOLL_CTL_ADD fail");
    }
}

void eventSet(int epfd,int event,int sockfd)
{
    struct epoll_event events;
    events.events=event;
    events.data.fd=sockfd;

    int ret = epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&events);
    if(ret!=0){
        perror("epoll_ctl EPOLL_CTL_ADD fail");
    }
}


void eventDel(int epfd,int event,int sockfd)
{
    struct epoll_event events;
    events.events=event;
    events.data.fd=sockfd;

    int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&events);
    if(ret!=0){
        perror("epoll_ctl EPOLL_CTL_DEL fail");
    }
}