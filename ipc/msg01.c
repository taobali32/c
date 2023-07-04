#include <stdio.h>
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

    struct msqid_ds buf;


    int ret2 = msgctl(msgid,IPC_RMID,NULL);

    printf("msgctl ret = %d\r\n",ret2);

    printf("msg_qnum=%ld\r\n",buf.msg_qnum);

    return 0;
}