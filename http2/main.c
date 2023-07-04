#include "service.h"

Service server = {
        .ip="0.0.0.0",
        .port=9501,
        .backlog=1024,
        .InitSocket=initSocket,
        .Bind=Bind,
        .Listen=Listen,
        .Run=Run
};

int main(){
    signal(SIGUSR1,SIG_IGN);

    printf("tcp server start,pid=%d\r\n",getpid());

    server.InitSocket();
    server.Bind();
    server.Listen();
    server.Run();

    while (1){
        sleep(1);
    }
}