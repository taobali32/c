#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{

    umask(0);

    pid_t pid = fork();
    if (pid>0){

        exit(0);
    }

    //调用完setsid后，它会变成会话首进程，组长进程，不带控制终端
    if(setsid()==-1){

        exit(0);
    }

//	调用setsid后，此时它是会话首进程
    //彻底的断开控制终端,让会话首进程退出
    pid = fork();
    if(pid>0){

        exit(0);
    }
    //当前进程就是一个普通的子进程
    //
    struct rlimit rlim;
    int ret = getrlimit(RLIMIT_NOFILE,&rlim);

    for(int i=0;i<rlim.rlim_max;i++){

        close(i);//0,1,2,3,4,
    }

    chdir("/");

    //打开的文件描述符返回是0
    int fd = open("/dev/null",O_RDWR);

    int fd1 = dup(0);
    int fd2 = dup(0);

    if(fd!=0&&fd1!=1&&fd2!=2){

        exit(0);
    }


    pid = fork();

    if(pid==0){

        while(1){

            printf("child pid test\r\n");
            sleep(2);
        }
        exit(0);
    }

    while(1){

        printf("parent pid test\r\n");
        sleep(2);

    }

    exit(0);


}
