#include "service.h"

Service server = {
        .ip="0.0.0.0",
        .port=9501,
        .backlog=1024,
        .thread_num=2,
        .InitSocket=initSocket,
        .Bind=Bind,
        .ix=0,
        .Listen=Listen,
        .Run=Run
};

int main(){
    signal(SIGUSR1,SIG_IGN);
    //  不然会出现 writeData fail Connection reset by peer
    //  多刷新几次就出现了!
    signal(SIGPIPE,SIG_IGN);

    printf("tcp server start,pid=%d\r\n",getpid());

    server.InitSocket();
    server.Bind();
    server.Listen();
    server.Run();

    while (1){
        sleep(1);
    }
}