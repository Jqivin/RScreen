#include "work.h"
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>
#define ARGC 10
#define NO_FILENAME "err#no filename"
#define NOT_EXIST "err#file not exists"
#define CREAT_ERR "err#file can't create"
#define UP_ERR "err#serve error in writing"
struct message
{
    struct event_base *base;
    struct event *c_ev;
    int file_fd; //当前打开的文件
    int curr_size;
    // int file_size;
};

void recv_cb(int fd, short ev, void *arg);

//注册登录///////////
//连接数据库
MYSQL *con;
MYSQL_RES *res_ptr; //存储查询结果
bool connectToMysql()
{
    int ret = 0;
    ////////////////////////////////////////
    // if(!con)
    //     memset(con,0,sizeof(MYSQL));
    /////////////////////////////////////////
    con = mysql_init(NULL); //初始化
    if (!con)
    {
        printf("mysql_init is failed!");
        return false;
    }

    //连接数据库
    con = mysql_real_connect(con, "localhost", "root", "123456", "RCloud", 0, NULL, 0);
    if (!con)
    {
        return false;
    }
    return true;
}

//登录函数
void login_fun(int c, char *userName, char *passWord, struct message *ms)
{
    //判断用户名或密码为空
    if (userName == NULL || passWord == NULL)
    {
        send(c, NO_FILENAME, strlen(NO_FILENAME), 0);
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);
        return;
    }
    //判断用户名密码是否正确
    //连接数据库
    if (connectToMysql() == false)
    {
        // mysql_close(con);
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);

        return;
    }

    //判断用户名和密码是否正确
    char str[128] = "select * from user where username='";
    strcat(str, userName);
    strcat(str, "' and passwd='");
    strcat(str, passWord);
    strcat(str, "'");
    int ret = mysql_query(con, str);
    if (ret == 0)
    {
        /////////////////////////////////////////////////////////
        // if(!res_ptr)
        //     memset(res_ptr,0,sizeof(MYSQL_RES));   //清理内存
        //////////////////////////////////////////////////////////
        res_ptr = mysql_store_result(con);
        if (res_ptr)
        {
            int rows = (int)mysql_num_rows(res_ptr);
            //查询到了
            if (rows == 1)
            {
                char *buff = "ok#";
                send(c, buff, strlen(buff), 0);
            }
            else
            {
                char *buff = "err#";
                send(c, buff, strlen(buff), 0);
            }
        }
    }
    mysql_close(con);
    event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
    event_add(ms->c_ev, NULL);
    return;
}
//注册函数
void register_fun(int c, char *userName, char *passWord, struct message *ms)
{
    //判断用户名或密码为空
    if (userName == NULL || passWord == NULL)
    {
        send(c, NO_FILENAME, strlen(NO_FILENAME), 0);
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);
        return;
    }
    //判断用户名密码是否正确
    //连接数据库
    if (connectToMysql() == false)
    {
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);
        return;
    }

    //判断用户名是否已经存在
    char str[128] = "select * from user where username='";
    strcat(str, userName);
    strcat(str, "' and passwd='");
    strcat(str, passWord);
    strcat(str, "'");

    // printf("register str:%s",str);
    int ret = mysql_query(con, str);
    if (ret == 0)
    {
        res_ptr = mysql_store_result(con);
        if (res_ptr)
        {
            int rows = (int)mysql_num_rows(res_ptr);
            //查询到了
            if (rows == 1)
            {
                char *buff = "exist";
                send(c, buff, strlen(buff), 0);
                mysql_close(con);
                printf("username existed\n");
                event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
                event_add(ms->c_ev, NULL);
                return;
            }
            else if (rows == 0)
            {
                char str[128] = "INSERT INTO user(username,passwd) VALUES('";
                strcat(str, userName);
                strcat(str, "','");
                strcat(str, passWord);
                strcat(str, "')");

                int ret = mysql_query(con, str);
                if (!ret)
                {
                    char *buff = "ok###";
                    send(c, buff, strlen(buff), 0);
                }
                else
                {
                    char *buff = "err##";
                    send(c, buff, strlen(buff), 0);
                }
            }
        }
    }
    mysql_close(con);
    event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
    event_add(ms->c_ev, NULL);
}

////////////////////
char *read_cmd(char *myargv[], char buff[]) //命令解析
{
    char *ptr = NULL;
    char *s = strtok_r(buff, " ", &ptr);
    int i = 0;
    while (s != NULL)
    {
        myargv[i++] = s;
        s = strtok_r(NULL, " ", &ptr);
    }
    return myargv[0];
}

void send_data(int c, short ev, void *arg)
{

    struct message *ms = (struct message *)arg;
    if (ev & EV_READ) //客户端异常崩掉的情况
    {
        char buff_status[128] = {0};
        int num = recv(c, buff_status, 127, 0);
        //异常终止的情况
        if (num <= 0)
        {
            close(ms->file_fd);
            close(c);
            event_free(ms->c_ev);
            free(ms);
            printf("close err\n");
            return;
        }
        //服务器收到客户端发的停止消息
        if (strcmp(buff_status, "stop") == 0)
        {
            close(ms->file_fd);
            event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
            event_add(ms->c_ev, NULL);
            return;
        }
    }

    if (ev & EV_WRITE)
    {
        char tmp[1024] = {0};
        int n = read(ms->file_fd, tmp, 1024);
        if (n == 0)
        {
            //文件发完
            event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms); //不要写事件了  ??应该是recv_cb吧
            event_add(ms->c_ev, NULL);
            return;
        }
        //发数据
        send(c, tmp, n, 0);
        event_assign(ms->c_ev, ms->base, c, EV_READ | EV_WRITE, send_data, ms); //这个读时间是为了安全，防止客户端异常终止
        event_add(ms->c_ev, NULL);
    }
}
//客户端回复服务端的时候的事件
void recv_status(int c, short ev, void *arg)
{
    struct message *ms = (struct message *)arg;
    if (ev & EV_READ)
    {
        char buff_status[128] = {0};
        int num = recv(c, buff_status, 127, 0);
        if (num <= 0)
        {
            close(c);
            event_free(ms->c_ev);
            free(ms);
            printf("one client close\n");
            return;
        }

        if (strcmp(buff_status, "ok#") == 0) //客户端要下载
        {
            event_assign(ms->c_ev, ms->base, c, EV_READ | EV_WRITE, send_data, ms); //要不要无所谓吧？？？
            event_add(ms->c_ev, NULL);
        }
        else
        {
            //客户端不下载了
            close(ms->file_fd); //关闭文件
            event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
            event_add(ms->c_ev, NULL);
        }
    }
}

//给客户端发送文件
void send_file(int c, char *filename, struct message *ms)
{
    if (filename == NULL) //文件名不存在
    {
        send(c, NO_FILENAME, strlen(NO_FILENAME), 0);
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);
        return;
    }
    //打开文件
    int file_fd = open(filename, O_RDONLY); //认为在当前路径
    if (file_fd == -1)
    {
        send(c, NOT_EXIST, strlen(NOT_EXIST), 0);
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);
        return;
    }
    ms->file_fd = file_fd;                      //记录文件描述付的值
    int filesize = lseek(file_fd, 0, SEEK_END); //得到文件的大小
    lseek(file_fd, 0, SEEK_SET);                //将指针挪到文件的开头

    //组装报文
    char head_buff[128] = {0};
    sprintf(head_buff, "ok#%d", filesize);
    send(c, head_buff, strlen(head_buff), 0); // send ： ok#filesize

    event_assign(ms->c_ev, ms->base, c, EV_READ, recv_status, ms);
    event_add(ms->c_ev, NULL);
}
void recv_data(int c, short ev, void *arg)
{
    struct message *ms = (struct message *)arg;
    if (ev & EV_READ)
    {
        char data[1024] = {0};
        int num = recv(c, data, 1024, 0);
        if (num <= 0) //客户端关闭
        {
            close(c);
            ms->curr_size = 0;
            free(ms->c_ev);
            free(ms);
            return;
        }
        int n = write(ms->file_fd, data, num);
        // if(n == -1)
        // {
        //     send(c,UP_ERR,strlen(UP_ERR),0);
        //     event_assign(ms->c_ev,ms->base,c,EV_READ,recv_cb,ms);
        //     event_add(ms->c_ev,NULL);
        //     return;
        // }
        // ms->curr_size += num;
        // //组装报文
        // char head_buff[128] = {0};
        // sprintf(head_buff,"ok#%d",ms->curr_size);
        // send(c,head_buff,strlen(head_buff),0);
        if (n < 1024)
        {
            //文件即将写完
            ms->curr_size += num;
            send(c, &ms->curr_size, 4, 0);
            ms->curr_size = 0;
            event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
            event_add(ms->c_ev, NULL);
            return;
        }
        ms->curr_size += num;
        send(c, &ms->curr_size, 4, 0);

        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_data, ms);
        event_add(ms->c_ev, NULL);
    }
}
void recv_up_status(int c, short ev, void *arg)
{
    struct message *ms = (struct message *)arg;
    if (ev & EV_READ) //有读事件产生，即c的接收缓冲区有数据产生了，也就是客户端send了
    {
        int filesize = 0;
        int num = recv(c, &filesize, 4, 0);
        if (num <= 0) //客户端关闭
        {
            close(c);
            free(ms->c_ev);
            free(ms);
            return;
        }

        int ser_file_size = lseek(ms->file_fd, 0, SEEK_END);
        lseek(ms->file_fd, 0, SEEK_SET);

        //服务器端已经有这个文件了，可以实现秒传
        if (ser_file_size == filesize)
        {
            send(c, "FINISHED", 8, 0);
            event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
            event_add(ms->c_ev, NULL);
            return;
        }
        else //服务器上没有要上传的文件，需要接收客户端的文件，并保存
        {
            send(c, "ok#", 3, 0);
            event_assign(ms->c_ev, ms->base, c, EV_READ, recv_data, ms);
            event_add(ms->c_ev, NULL);
            return;
        }
    }
}
//上传文件到服务器
void up_file(int c, char *filename, void *arg)
{
    struct message *ms = (struct message *)arg;
    if (filename == NULL)
    {
        //没有传进来文件名
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);
        return;
    }
    int file_fd = open(filename, O_RDWR | O_CREAT, 0777);
    if (file_fd == -1)
    {
        send(c, CREAT_ERR, strlen(CREAT_ERR), 0);
        event_assign(ms->c_ev, ms->base, c, EV_READ, recv_cb, ms);
        event_add(ms->c_ev, NULL);
        return;
    }
    send(c, "ok#", 3, 0); //给客户端回应这边准备好了
    ms->file_fd = file_fd;
    event_assign(ms->c_ev, ms->base, c, EV_READ, recv_up_status, ms);
    event_add(ms->c_ev, NULL);
}
void recv_cb(int fd, short ev, void *arg)
{
    struct message *ms = (struct message *)arg;
    if (EV_READ & ev)
    {
        char buff[128] = {0};
        int n = recv(fd, buff, 127, 0); //收到的是客户端发给服务端的报文 get a.c
        if (n <= 0)
        {
            close(fd);
            event_free(ms->c_ev);
            free(ms);
            printf("one client close\n");
            return;
        }
        char *myargv[ARGC] = {0}; //存放解析完命令的结果 如ls : myargv[0] = "ls"; rm: myargv[0] = "rm" myargv[1] = a.c;
        char *cmd = read_cmd(myargv, buff);
        if (cmd == NULL) //输入的是空格的情况,重新注册事件
        {
            event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
            event_add(ms->c_ev, NULL);
            send(fd, "ok#", 3, 0);
            return;
        }
        if (strcmp(cmd, "get") == 0)
        {
            //下载
            send_file(fd, myargv[1], ms);
            return;
        }
        else if (strcmp(cmd, "up") == 0)
        {
            //上传
            up_file(fd, myargv[1], ms);
            return;
        }
        else if (strcmp(cmd, "login") == 0)
        {
            //登录
            // login_fun(fd,myargv[1],myargv[2],ms);
            if (connectToMysql() == false)
            {
                printf("connect mysql false\n");
                // mysql_close(con);
                event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
                event_add(ms->c_ev, NULL);
                return; // return之前一定要再注册一下
            }
            printf("connect mysql is ok\n");

            //判断用户名和密码是否正确
            char str[128] = "select * from user where username='";
            strcat(str, myargv[1]);
            strcat(str, "' and passwd='");
            strcat(str, myargv[2]);
            strcat(str, "'");
            int ret = mysql_query(con, str);
            //printf("buff is %s\n", str);
            if (ret == 0)
            {
                //printf(" mysql_query is ok\n");
                res_ptr = mysql_store_result(con);
                //printf("mysql_store_result is ok\n");
                if (res_ptr)
                {
                    int rows = (int)mysql_num_rows(res_ptr);
                    //查询到了
                    if (rows == 1)
                    {
                        char *buf = "ok#";
                        send(fd, buf, strlen(buf), 0);
                    }
                    else
                    {
                        char *buf = "err#";
                        send(fd, buf, strlen(buf), 0);
                    }
                }
                else
                {
                    printf("mysql_store_result err\n");
                    event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
                    event_add(ms->c_ev, NULL);
                    return; // return之前一定要再注册一下
                }
            }
            //printf("login1\n");
            event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
            event_add(ms->c_ev, NULL);
            //printf("login2\n");
            return; // return之前一定要再注册一下
        }
        else if (strcmp(cmd, "register") == 0)
        {
            //注册
            // register_fun(fd,myargv[1],myargv[2],ms);
            //判断用户名密码是否正确
            //连接数据库
            if (connectToMysql() == false)
            {
                printf("connect mysql false\n");
                event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
                event_add(ms->c_ev, NULL);
                return; // return之前一定要再注册一下
            }

            //判断用户名和密码是否正确
            char str[128] = "select * from user where username='";
            strcat(str, myargv[1]);
            strcat(str, "' and passwd='");
            strcat(str, myargv[2]);
            strcat(str, "'");
            int ret = mysql_query(con, str);
            //printf("buff is %s\n", str);
            if (ret == 0)
            {
                res_ptr = mysql_store_result(con);
                if (res_ptr)
                {
                    int rows = (int)mysql_num_rows(res_ptr);
                    //查询到了
                    if (rows == 1)
                    {
                        char *buff = "exist";
                        send(fd, buff, strlen(buff), 0);
                        mysql_close(con);
                        //printf("username existed\n");
                        event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
                        event_add(ms->c_ev, NULL);
                        return;
                    }
                    else if (rows == 0)
                    {
                        char str[128] = "INSERT INTO user(username,passwd) VALUES('";
                        strcat(str, myargv[1]);
                        strcat(str, "','");
                        strcat(str, myargv[2]);
                        strcat(str, "')");

                        int ret = mysql_query(con, str);
                        if (!ret)
                        {
                            char *buff = "ok###";
                            send(fd, buff, strlen(buff), 0);
                        }
                        else
                        {
                            char *buff = "err##";
                            send(fd, buff, strlen(buff), 0);
                        }
                    }
                }
                event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
                event_add(ms->c_ev, NULL);
                return;
            }
        }
        else
        {
            int pipefd[2];
            pipe(pipefd); //创建管道
            pid_t pid = fork();
            if (pid == -1) //如果创建失败，给客户端返回一个信息提示错误
            {
                send(fd, "ok#fork error", 13, 0);
                event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms);
                event_add(ms->c_ev, NULL);
                return; // return之前一定要再注册一下
            }
            if (pid == 0)
            {
                close(pipefd[0]);
                dup2(pipefd[1], 1);  //将命令执行结果如ls执行的结果打印到管道中，原来是打印到标准输出，dup2之后就打印到了管道中
                dup2(pipefd[1], 2);  //把标准错误也打到管道中
                execvp(cmd, myargv); //进程替换的功能，将子进程替换成cmd命令如rm myargv是参数 如 rm a.c b.c
                printf("error:%s not found!", cmd);
                exit(0);
            }
            close(pipefd[1]); //关写
            char readpipe[1024] = {"ok#"};
            wait(NULL);
            read(pipefd[0], readpipe + 3, 1021); //从管道中读到数组中
            close(pipefd[0]);
            send(fd, readpipe + 3, strlen(readpipe), 0); //发送给客户端
        }
        event_assign(ms->c_ev, ms->base, fd, EV_READ, recv_cb, ms); //让每次响应
        event_add(ms->c_ev, NULL);
    }
}

void accept_cb(int fd, short ev, void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    if (ev & EV_READ) //看传进来的是不是读事件，因为accept处理读事件
    {
        struct sockaddr_in caddr;
        int len = sizeof(caddr);

        int c = accept(fd, (struct sockaddr *)&caddr, &len); //产生新的描述符
        if (c < 0)
        {
            return;
        }
        //这里没有用event_new()
        struct event *c_ev = (struct event *)malloc(sizeof(struct event));
        if (c_ev == NULL)
        {
            close(c);
            return;
        }
        struct message *ms = (struct message *)malloc(sizeof(struct message));
        ms->base = base;
        ms->c_ev = c_ev;
        event_assign(c_ev, base, c, EV_READ, recv_cb, ms); //不写永久性事件，因为可能还有写事件
        event_add(c_ev, NULL);
    }
}
