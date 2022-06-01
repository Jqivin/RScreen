#include "work.h"

//#define IPSTR "127.0.0.1"
#define IPSTR "192.168.221.128"
#define PORT 6000
#define LIS_MAX 5

int create_socket();
int main()
{
    int sockfd = create_socket();
    assert(sockfd != -1);

    struct event_base* base = event_init();//创建libevent实例
    assert(base != NULL);
    struct event* sock_ev = event_new(base,sockfd,EV_READ|EV_PERSIST,accept_cb,base); //为sockfd创建事件
    assert(sock_ev != NULL);
    event_add(sock_ev,NULL);

    event_base_dispatch(base);  //阻塞住，调用IO函数
    event_free(sock_ev);
    event_base_free(base);
    return 0;
}
int create_socket()
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        return -1;
    }
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = inet_addr(IPSTR);

    int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(res == -1)
        return -1;
    res = listen(sockfd,LIS_MAX);
    if(res == -1)
        return -1;
    return sockfd;
}


