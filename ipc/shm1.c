#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>



int main()
{
//    int shmid = shmget(0x2345,1024,IPC_CREAT|IPC_EXCL|0666);

    int shmid = shmget(0x2345,0,0);
    printf("shmid=%d,errno=%d,error=%s\r\n",shmid,errno, strerror(errno));


    char *msg = (char*) shmat(shmid,NULL,0);

    memcpy(msg,"china",5);


    int ret = shmdt((void *)msg);
    printf("shmdt ret=%d\r\n",ret);

    return 0;
}
