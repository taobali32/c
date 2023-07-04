//
// 向线程传递参数
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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Stack{
    int data;
    struct Stack *next;
} Stack;

struct Stack *data;

Stack* push(Stack *data,int elem){
    struct Stack *temp = (Stack*) malloc(sizeof(struct Stack));

    if(temp!= NULL){
        temp->data= elem;
        temp->next = data;
        data = temp;
    }else{
        printf("内存分配失败，数据插入失败！\n");
    }

    return data;
}


Stack* pop(Stack *data,int *elem){
    struct Stack *temp = data;
    if (data != NULL){
        data = data->next;
        *elem = temp->data;
        free(temp); // 释放弹出的节点内存
    }else{
        printf("栈为空！\n");
    }

    return data;
}

void *mythread1(){

    for (int i = 0; i < 10000; ++i) {
        pthread_mutex_lock(&mutex);

        printf("插入数据第%d次操作,elem=%d\r\n",i,i);

        data = push(data,i);

        pthread_mutex_unlock(&mutex);
    }
    
    pthread_exit( (void *)3 );
}

void *mythread2(){
    int elem;

    for (int i = 0; i < 10000; ++i) {

        pthread_mutex_lock(&mutex);
        if (pthread_mutex_trylock(&mutex) == 0){
            data = pop(data,&elem);

            if (elem){
                printf("第%d次操作,elem=%d\r\n",i,elem);
            }
        }
//        data = pop(data,&elem);
//
//        if (elem){
//            printf("第%d次操作,elem=%d\r\n",i,elem);
//        }

        pthread_mutex_unlock(&mutex);

    }

    pthread_exit((void *)3);
}

int main(){
    //  互吃锁，  用于多线程数据同步，
    pthread_t tid,tid2;

    pthread_create(&tid,NULL,mythread1,NULL);
    pthread_create(&tid2,NULL,mythread2,NULL);

    int *status,*status2;
    pthread_join(tid,&status );
    pthread_join(tid2,&status2 );

    return 0;
}

/**
*   data = push(data,1);
    data = push(data,2);
    data = push(data,3);

    int elem;
    data = pop(data,&elem);

    printf("elem=%d\r\n",elem);
*/