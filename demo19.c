#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int signo){
    printf("signo=%d\r\n",signo);
}


int main() {

    printf("pid=%d\r\n",getpid());
    /**
     * 在主函数中，分别调用signal函数来设置三种不同类型的信号处理。
     * SIGINT表示当用户按下CTRL+C键时会调用signal_handler函数，
     * SIGUSR1表示忽略该信号，
     * SIGUSR2表示使用默认信号处理,大部份进程被终止掉。
     */
    signal(SIGINT,signal_handler);

    signal(SIGUSR1,SIG_IGN);

    signal(SIGUSR2,SIG_DFL);


    while (1){
        printf("test\r\n");
        sleep(10);
    }
    return 0;
}
