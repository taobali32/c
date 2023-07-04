//
// Created by macos on 2023/6/24.
// man 7 thread
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <string.h>
#include <arpa/inet.h>

#include <pthread.h>

#include <unistd.h>

void *mythread(){
    printf("i am child\r\n");
    pthread_exit( (void *)3 );
}

int main(){
    pthread_t tid;

    pthread_create(&tid,NULL,mythread,NULL);

    int *status;
    pthread_join(tid,&status );
    printf("exit child thread status =%d\r\n",(int )status);

    return 0;
}
