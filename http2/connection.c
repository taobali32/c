//
// Created by Administrator on 2021/8/10 0010.
//
#include "service.h"
#include "connection.h"
//client 是struct connection 结构体指针变量 【c++/py/php】

int recvData(connection *client)
{
    if(client->sockfd==0){
        return -1;
    }
    //目前缓冲区剩下长度
    int remain_len = client->recv_max_bytes-client->recv_last;
    if(remain_len>0){

        //从指定位置接收数据
        char *data = client->recv_buffer+client->recv_last;

        int recvBytes = recv(client->sockfd,data,remain_len,0);

        if(recvBytes==0){//对端已经关闭

            return -1;
        }
        if(recvBytes<0){
            printf("recvData recv fail  %s\r\n",strerror(errno));
            return -1;
        }
        //接收字节累加
        client->recv_last+=recvBytes;
        client->recv_buffer[client->recv_last]=0;
        return recvBytes;

    }else{

        client->recv_buffer_full++;
    }

    return 0;
}
int pushData(connection *client,char *data,int len)
{
    if(client->sockfd==0){
        return -1;
    }

    if(client->send_last+len<client->send_max_bytes){

        memcpy(client->send_buffer+client->send_last,data,len);

        client->send_last+=len;

        if(client->send_last>=client->send_max_bytes){

            client->send_buffer_full++;
        }

    }else{

        client->send_buffer_full++;
    }

    return 0;

}
int writeData(connection *client)
{
    if(client->sockfd==0){
        return -1;
    }


    if(client->send_last>0){

        int sendBytes = send(client->sockfd,client->send_buffer,client->send_last,0);

        if(sendBytes==0){
            printf("writeData fail %s\r\n",strerror(errno));
            return -1;
        }
        else if(sendBytes<0){
            printf("writeData fail %s\r\n",strerror(errno));
            return -1;
        }
        else if (sendBytes==client->send_last){

            //一次性发送完
            memset(client->send_buffer,0, sizeof(client->send_buffer));
            client->send_last=0;
            client->send_buffer_full=0;
        }else{

            //发送一半的情况下
            client->send_last-=sendBytes;
            memcpy(client->send_buffer,client->send_buffer+sendBytes,client->send_last);
            return sendBytes;
        }

    }
    //
//    printf("走到了%d\r\n\r\n",client->sockfd);
//    close(client->sockfd);

    return 0;

}

void popMessage(connection *client)
{

    if(client->recv_last>0){

        //client->recv_last 表示当前连接接收缓冲区已经接收的总字节数  100Kb
        //client->data->header_len+client->data->body_len 这个表示的是一条完整的HTTP 报文，假如这完整的HTTP报文字节数为10KB
        //
        int length = client->data->header_len+client->data->body_len;
        int remain_len = client->recv_last-length;

        if(remain_len>0){

            memcpy(client->recv_buffer,client->recv_buffer+length,remain_len);
        }
        client->recv_last=remain_len;

        if (client->recv_buffer_full>0){

            client->recv_buffer_full--;
        }
    }
}

int hasMessage(connection *client)
{
    if(client->sockfd==0){
        return -1;
    }

    char *temp = strstr(client->recv_buffer,"\r\n\r\n");
    if(!temp){

        return -1;
    }

    //指针是可以进行
    //temp = \r\n\r\nprice=199&goods=iphone
    //temp 它的位置 x
    //client->recv_buffer 它的位置是0

    int header_len = temp-client->recv_buffer;

    header_len+=4;
    int body_len = 0;

    char *content_len = strstr(client->recv_buffer,"Content-Length: ");
    if (content_len){
        //如果存在，表明有请求体

        //content_len = Content-Length: 22\r\n\r\nprice=199&goods=iphone
        content_len+=16;
        //content_len = 22\r\n\r\nprice=199&goods=iphone

        char *c = strchr(content_len,'\r');
        //c = \r\n\r\nprice=199&goods=iphone

        int length = c - content_len;
        //1000,000 1M的数据量  1M=1024KB  1KB=1024Bytes
        if(length>7){
            //请求行+请求头超过了1M的数据量，太多，不允许
            return -1;
        }

        char body_str_len[7]={0};

        strncpy(body_str_len,content_len,length);

        body_len = atoi(body_str_len);

        if(header_len+body_len>client->recv_last){
            //表示消息还没有接收完整
            return -1;
        }

        if(header_len+body_len>client->recv_max_bytes){

            //接收的实际内容超出了缓冲区的规定大小
            return -1;
        }

    }


    client->data = (http*)malloc(sizeof(http));
    client->data->body_len=body_len;
    client->data->header_len=header_len;

    return header_len;


}
int http_response_file(connection *client,char *data,int type,int len)
{
    char *response = (char*)malloc(sizeof(char)*len+200);
    strcat(response,"HTTP/1.1 200 OK\r\n");

    if (type==0){
        strcat(response,"Content-Type: text/html\r\n");

    }
    if (type==1){
        strcat(response,"Content-Type: image/jpeg\r\n");

    }

    char body_len[50]={0};
    sprintf(body_len,"Content-Length: %d\r\n",len);

    strcat(response,body_len);
    strcat(response,"\r\n");
    //strcat(response,data);
    //strcat 拼装数据时，遇到\0就会截断

    pushData(client,response,strlen(response));
    pushData(client,data,len);

    return 0;
}
int http_response(connection *client,char *data,int type)
{
    char *response = (char*)malloc(sizeof(char)*strlen(data)+200);
    strcat(response,"HTTP/1.1 200 OK\r\n");

    if (type==0){
        strcat(response,"Content-Type: text/html\r\n");

    }
    if (type==1){
        strcat(response,"Content-Type: application/x-jpg\r\n");

    }

    char body_len[50]={0};
    sprintf(body_len,"Content-Length: %d\r\n",strlen(data));
    strcat(response, "Connection: keep-alive\r\n");
    strcat(response,body_len);
    strcat(response,"\r\n");
    strcat(response,data);

    pushData(client,response,strlen(response));

    return 0;
}

int http_parse(connection *client)
{

    if(client->data->header_len<=0){
        return 0;
    }

    char *data = client->recv_buffer;

    client->data->get = NULL;
    client->data->post = NULL;
    client->data->header = NULL;

    data[client->data->header_len-1]='\0';

    char *temp = strstr(data,"\r\n");

    temp[0]='\0';


    /****************请求行解析**********************/
    char method[10];
    char uri[128];
    char version[10];

    data = splitStr(data," ",method);
    data = splitStr(data," ",uri);
    splitStr(data," ",version);

    client->data->method = (char*)malloc(sizeof(char)* sizeof(method));
    client->data->uri = (char*)malloc(sizeof(char)* sizeof(uri));
    client->data->version = (char*)malloc(sizeof(char)* sizeof(version));

    strcpy(client->data->method,method);
    strcpy(client->data->uri,uri);
    strcpy(client->data->version,version);

    /****************请求行解析**********************/

    //查询字符串解析uri=/index.html?name=china&age=8888
    char *query = splitStr(uri,"?",uri);
    if (query!=""){

        strcpy(client->data->uri,uri);

        char query_field[120];
        client->data->get = (kv*)malloc(sizeof(kv)*50);
        int query_idx=0;

        while (1){

            query = splitStr(query,"&",query_field);

            if(query_field){

                char key[30];

                char *val = splitStr(query_field,"=",key);

                kv obj;
                obj.key =  (char*)malloc(sizeof(char)*strlen(key));
                obj.val =  (char*)malloc(sizeof(char)*strlen(val));
                strcpy(obj.key,key);
                strcpy(obj.val,val);

                client->data->get[query_idx] = obj;
                query_idx++;
            }
            if (query==""){
                break;
            }
        }
    }

    /****************请求头解析**********************/

    temp+=2;

    char header[200];
    client->data->header = (kv*)malloc(sizeof(kv)*50);
    int header_idx=0;
    while (1){

        temp = splitStr(temp,"\r\n",header);

        if(strcmp("\0\n",temp)!=0){

            char key[50];

            char *val = splitStr(header,": ",key);

            kv obj;
            obj.key =  (char*)malloc(sizeof(char)*strlen(key));
            obj.val =  (char*)malloc(sizeof(char)*strlen(val));
            strcpy(obj.key,key);
            strcpy(obj.val,val);

            client->data->header[header_idx] = obj;
            header_idx++;

        }
        if(temp==""){
            break;
        }
        memset(header,0, sizeof(header));

    }

    //有请求体才解析
    if(client->data->body_len>0){

        char *body = client->recv_buffer+client->data->header_len;
        char body_field[120];
        client->data->post = (kv*)malloc(sizeof(kv)*50);
        int body_idx=0;

        while (1){

            body = splitStr(body,"&",body_field);

            if(body_field){

                char key[50];

                char *val = splitStr(body_field,"=",key);

//                if (strlen(val)>10){
//
//                    printf("%s 的值太长，我不要了 %d\r\n",key,strlen(val));
//
//                }
                kv obj;
                obj.key =  (char*)malloc(sizeof(char)*strlen(key));
                obj.val =  (char*)malloc(sizeof(char)*strlen(val));
                strcpy(obj.key,key);
                strcpy(obj.val,val);

                client->data->post[body_idx] = obj;
                body_idx++;
            }
            if (body==""){
                break;
            }
        }
    }

    return 1;


}

char *getHeader(connection *client,char *key){

    int ix=0;
    if (client->data->header==NULL){
        return "";
    }
    for(ix;client->data->header[ix].key;ix++){

        if(strcmp(client->data->header[ix].key,key)==0){

            return client->data->header[ix].val;
        }
    }
    return "";

}
char *getPost(connection *client,char *key){

    int ix=0;
    if (client->data->post==NULL){
        return "";
    }
    for(ix;client->data->post[ix].key;ix++){

        if(strcmp(client->data->post[ix].key,key)==0){

            return client->data->post[ix].val;
        }
    }
    return "";

}

char *getQuery(connection *client,char *key){

    int ix=0;
    if (client->data->get==NULL){
        return "";
    }
    for(ix;client->data->get[ix].key;ix++){

        if(strcmp(client->data->get[ix].key,key)==0){

            return client->data->get[ix].val;
        }
    }
    return "";

}
char *splitStr(char *data,char *split,char *res){

    char *p = strstr(data,split);
    if(!p){
        if(res!=NULL)
        {
            strcpy(res,data);
        }
        return "";
    }
    p[0]='\0';
    if(res!=NULL){
        strcpy(res,data);
    }

    p+=strlen(split);

    return p;
}