#include "service.h"
#include <pthread.h>

//  这行代码创建了一个静态的互斥锁，并使用宏PTHREAD_MUTEX_INITIALIZER对其进行了初始化。
//  互斥锁用于保护临界区，确保同一时间只有一个线程可以访问临界区内的代码。
static pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;

//  这行代码创建了一个静态的条件变量，并使用宏PTHREAD_COND_INITIALIZER对其进行了初始化。
//  条件变量用于线程间的同步，可以让一个线程等待另一个线程满足某个条件后再继续执行。
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int stauts=0;

void createThread(void *(*func)(void *), void *arg) {
    pthread_mutex_lock(&mutex);

    pthread_t tid;

    if (pthread_create(&tid,NULL,func,arg)){
        perror("pthread_create fail \r\n");
        exit(0);
    }
    pthread_detach(tid);

    pthread_mutex_unlock(&mutex);
}


void stopListenThread(Service *server)
{
    pthread_mutex_lock(&mutex);

    server->run_flag=0;
    stauts=0;
    while (stauts==0){

        //  等待条件变量cond满足，并释放互斥锁mutex。当条件变量满足时，线程会重新获取互斥锁并继续执行。
        pthread_cond_wait(&cond,&mutex);
    }

    pthread_mutex_unlock(&mutex);
}

void stopConnectThread(reactor *server)
{
    pthread_mutex_lock(&mutex);

    server->run_flag=0;
    stauts=0;
    while (stauts==0){

        pthread_cond_wait(&cond,&mutex);
    }

    pthread_mutex_unlock(&mutex);
}

void notifyThread()
{
    pthread_mutex_lock(&mutex);

    stauts=1;
    pthread_cond_broadcast(&cond);

    pthread_mutex_unlock(&mutex);
}
