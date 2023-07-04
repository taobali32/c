#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main()
{
    int semid = semget(0x2345, 1, IPC_CREAT | IPC_EXCL | 0666);
    // 获取信号量集，如果不存在则创建一个新的

    if(semid < 0) // 如果信号量集创建失败
    {
        semid = semget(0x2345,1,0);  // 尝试获取已经存在的信号量集

        if(semid < 0) // 如果获取已经存在的信号量集失败
        {
            perror("semget error"); // 输出错误信息
            exit(1);  // 退出程序
        }
    }
    else // 如果信号量集创建成功
    {

    }

    union semun
    {
        int val;  // 数值
        struct semid_ds *buf;  // semid_ds 结构体指针
        unsigned short *array;  // 用于设置信号量各个值的数组
        struct seminfo *__buf;  // seminfo 结构体指针
    } buf;

    buf.val = 10;  // 初始化为 10
    int ret = semctl(semid, 0, SETVAL, buf);  // 对信号量进行设置

    if(ret < 0) // 如果设置失败
    {
        perror("semctl error"); // 输出错误信息
        exit(1);  // 退出程序
    }

    int ret2 = semctl(semid, 0, GETVAL);  // 获取信号量当前值

    if(ret2 < 0) // 如果获取失败
    {
        perror("semctl error"); // 输出错误信息
        exit(1);  // 退出程序
    }

    printf("GETVAL ret = %d\n", ret2);

    return 0;
}
