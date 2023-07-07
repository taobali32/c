//
// Created by macos on 2023/7/4.
//

#ifndef C_THREAD_H
#define C_THREAD_H

extern void createThread(void *(*func)(void*),void *arg);
extern void stopListenThread(Service *server);
extern void notifyThread();
extern void stopConnectThread(reactor *server);
#endif //C_THREAD_H
