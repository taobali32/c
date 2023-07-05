#include "service.h"
#include <pthread.h>

static pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;

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
