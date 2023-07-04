#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/signal.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <strings.h>
#include <sys/stat.h>

typedef struct process{
    int pipefd[2];
    pid_t pid;
} process;

typedef struct proc{
    int proc_num;
    process *p;
    int idx;
} proc;


#define FIFOX "./fifox"

static int g_run = 1;

int pool(proc *p, int num);


void sig_handler(int signo){

    g_run =0;
    printf("pid %d receive signo %d\r\n",getpid(),signo);
}


int pool(proc *p, int num) {
    if(num <= 0){
        return 0;
    }

    signal(SIGINT,sig_handler);
    signal(SIGQUIT,sig_handler);
    signal(SIGTERM,sig_handler);
    signal(SIGUSR1,sig_handler);

    /**
     * typedef struct proc{
          int proc_num;
          process *p;
          int idx;
        } proc;
     */
    p->idx = 0; //  作用?
    p->proc_num = num;

    p->p = (process *)malloc(sizeof(process) * num);

    //  创建进程
    for (int i = 0; i < num; ++i) {
        //  创建管道：使用int pipe(int pipefd[2])函数创建一个管道，
        //  其中pipefd是一个整型数组，用于存储管道的两个文件描述符。
        //  pipefd[0]用于读取数据，pipefd[1]用于写入数据。

        /**
         *  int fd[2] = {0};
            int ret = pipe(fd);
            if (ret == 0){
            } else{
                printf("失败\n");
            }
         */
        if (pipe(p->p[i].pipefd)){
            exit(0);
        }

        p->p[i].pid = fork();

        //  进程创建失败
        if(p->p[i].pid<0){
            exit(0);
        }

        if(p->p[i].pid>0){
            //  pipefd[0]用于读取数据, 不用就关掉。
            close(p->p[i].pipefd[0]);
            continue;
        } else{
            // 子进程执行，
            p->idx = i;

            //  子进程又不需要写入.
            close(p->p[i].pipefd[1]);

            //  下面代码是对文件描述符pipefd的属性进行设置，使其变为非阻塞模式。
            //  拿到 pipefd[0]用于读取数据的fd，文件描述符
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
    //

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

    int status;
    pid_t pid;
    for(int i=0;i<num;i++){

        if((pid=waitpid(-1,&status,0))>0){

            printf("子进程 %d 退出，exit status=%d\r\n",pid,status);
        }
    }
    printf("master pid=%d exit\r\n",getpid());

}

int main(){
    proc proc1;

    pool(&proc1,2);
    return 0;
}

