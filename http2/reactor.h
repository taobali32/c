//
// Created by Administrator on 2021/8/9 0009.
//

#ifndef CLESSON_REACTOR_H
#define CLESSON_REACTOR_H
#include "connection.h"
typedef struct reactor{

    int epfd;
    int ix;
    struct connection *clients;
    struct connection *clientsBuffer;
    int current_client_num;
    int max_client_num;
    int run_flag;


}reactor;
extern void *CellEventLoop(void *arg);
extern void addConnection(reactor *cell,connection client);
#endif //CLESSON_REACTOR_H
