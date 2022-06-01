#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<assert.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<event.h>
#include<sys/wait.h>
#include<fcntl.h>

void accept_cb(int fd,short ev,void* arg);
