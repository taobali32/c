#include <stdio.h>
#include <sys/resource.h>

int main() {
    struct rlimit rlim,arlim;

    arlim.rlim_cur = 10240;
    arlim.rlim_max = 10240;

    setrlimit(RLIMIT_NOFILE,&arlim);

    int ret = getrlimit(RLIMIT_NOFILE,&rlim);

    printf("该进程最多能打开:%lu文件\r\n",rlim.rlim_max);

    return 0;
}
