#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// 信号覆盖默认中断动作, 默认动作大部份程序没了
// 不能覆盖kill的

void signal_handler(int signo){
    printf("signo=%d\r\n",signo);
}

int main() {

    printf("pid=%d\r\n",getpid());

    //  阻塞中断信号
    sigset_t  bset,pending_set;
    sigemptyset(&bset);
    sigaddset(&bset,SIGINT);    //
    sigprocmask(SIG_BLOCK,&bset,NULL);

    /**
     * @var sigaction
     */
    struct  sigaction act;

    act.sa_handler = signal_handler;
    //  信号集
    sigemptyset(&act.sa_mask);

    //  中断信号 SA_RESTART看笔记
//    act.sa_flags = 0;
    act.sa_flags = SA_RESTART;

    if(sigaction(SIGINT,&act,NULL)){
        printf("信号注册失败\r\n");
    }

    char buffer[128] = {0};

    int j = 0;
    while (1){

//        int byte = read(0,buffer,sizeof (buffer));

//        printf("byte=%d,errno=%d,error=%s\r\n",byte,errno,strerror(errno));

        sigpending(&pending_set);

        // 32 // 64 个信号 只有32个常用
        for (int i = 0; i < 32; ++i) {
            if(sigismember(&pending_set,i)){
                printf("1");
            } else{
                printf("0");
            }

        }

        printf("\n");

        sleep(1);

        if(j++ == 10){
            //  解除阻塞
            sigprocmask(SIG_UNBLOCK,&bset,NULL);
        }
    }

    return 0;
}
