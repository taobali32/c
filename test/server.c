//
// Created by Administrator on 2021/8/6 0006.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef struct process{
    int pipefd[2];
    pid_t pid;
}process;

typedef struct proc{
    int proc_num;
    process *p;
    int idx;
}proc;

static int g_run=1;
#define FIFOX "./fifox"

void sig_handler(int signo){

    g_run =0;
    printf("pid %d receive signo %d\r\n",getpid(),signo);
}
void pool(proc *p,int num)
{

    if(num<=0){
        return;
    }
    signal(SIGINT,sig_handler);
    signal(SIGQUIT,sig_handler);
    signal(SIGTERM,sig_handler);
    signal(SIGUSR1,sig_handler);



    p->idx=0;
    p->proc_num=num;
    p->p = (process*)malloc(sizeof(process)*num);

    for(int i=0;i<num;i++){

        if(pipe(p->p[i].pipefd)){
            exit(0);
        }

        p->p[i].pid = fork();

        if(p->p[i].pid<0){
            exit(0);
        }

        if(p->p[i].pid>0){

            close(p->p[i].pipefd[0]);
            continue;
        }else{
            /*******************child space****************/
            //signal(SIGINT,SIG_IGN);
            //signal(SIGQUIT,SIG_IGN);
            //signal(SIGUSR1,SIG_IGN);

            p->idx=i;
            close(p->p[i].pipefd[1]);

            int pipefd = p->p[i].pipefd[0];
            int option = fcntl(pipefd,F_GETFL);
            option=option|O_NONBLOCK;
            fcntl(pipefd,F_SETFL,option);

            char msg[1024]={0};
            printf("child pid=%d start\r\n",getpid());
            while (g_run){


                //  read会走系统中断。。
                int ret = read(pipefd,msg,sizeof(msg));
                //printf("ret %d \r\n",ret);
                if(ret==-1 && errno==EAGAIN){

                    continue;
                }
                if (ret==0){
                    break;
                }
                printf("pid %d read %d bytes,msg=%s\r\n",getpid(),ret,msg);

                memset(msg,'\0',sizeof(msg));
            }
            printf("child pid=%d exit\r\n",getpid());
            exit(0);
            /*******************child space****************/
        }
    }

    /*******************parent space****************/
    //    struct sigaction act;
    //    act.sa_handler = sig_handler;
    //    sigemptyset(&act.sa_mask);
    //
    //    act.sa_flags=SA_RESTART;
    //
    //    sigaction(SIGINT,&act,NULL);
    //    sigaction(SIGQUIT,&act,NULL);
    //    sigaction(SIGTERM,&act,NULL);
    //    sigaction(SIGUSR1,&act,NULL);


    if(access(FIFOX,F_OK)){
        mkfifo(FIFOX,0666);
    }
    char msg[1024]={0};
    int idx=0;
    printf("master pid=%d start\r\n",getpid());
    int fd = open(FIFOX,O_RDONLY);
    int option = fcntl(fd,F_GETFL);
    option=option|O_NONBLOCK;
    fcntl(fd,F_SETFL,option);

    while (g_run) {



        int ret = read(fd,msg,sizeof(msg));
        if(ret==-1 && errno==EAGAIN){

            continue;
        }
        if (ret==0){
            break;
        }

        if(ret>0){
            //算法是：轮询算法
            idx = p->idx++%p->proc_num;

            int pipefd = p->p[idx].pipefd[1];
            int option = fcntl(pipefd,F_GETFL);
            option=option|O_NONBLOCK;
            fcntl(pipefd,F_SETFL,option);

            write(pipefd,msg, sizeof(msg));
            printf("父进程通信轮询算法向  %d 发送数据\r\n",idx);

            if(strncasecmp("exit",msg,4)==0){
                break;
            }
        }

    }

    //    for(int i=0;i<num;i++){
    //
    //        kill(p->p[i].pid,SIGKILL);
    //    }

    int status;
    pid_t pid;
    for(int i=0;i<num;i++){

        if((pid=waitpid(-1,&status,0))>0){

            printf("子进程 %d 退出，exit status=%d\r\n",pid,status);
        }
    }
    printf("master pid=%d exit\r\n",getpid());
    /*******************parent space****************/
}


int main()
{
    proc proc1;
    pool(&proc1,2);
    return 0;
}
