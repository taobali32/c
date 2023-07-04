#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h> // 需要添加的头文件
#include <sys/ipc.h> // 需要添加的头文件

struct msgbuf{
    long mtype;
    char mtext[1024];
};


int main(){
    //  创建一个新的消息队列
//    int msgid = msgget(0x2345,IPC_CREAT|IPC_EXCL|0666);

    //  引用1个存在的消息队列
    int msgid = msgget(0x2345,0);

    printf("msgid=%d,error=%d,error=%s\r\n",msgid,errno, strerror(errno));

    struct msgbuf msg;

//    int ret = msgrcv(msgid,(void *)&msg, sizeof(msg.mtext),0,0);

    int ret = msgrcv(msgid,(void *)&msg, sizeof(msg.mtext),0,IPC_NOWAIT );

    printf("msgrcv ret = %d,mtext=%s,mtype=%ld,ENOMSG=%d,errno=%d\n",ret,msg.mtext,msg.mtype,ENOMSG,errno);

    return 0;
}