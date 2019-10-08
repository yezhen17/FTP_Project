#include <sys/socket.h>
#include <sys/select.h>

#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOT_LOGGED_IN 0
#define LOGGED_IN 2
#define NO_CONNECTION 0
#define PASV_MODE 1
#define PORT_MODE 2
#define LISTENING 3
#define READY_TO_CONNECT 4
#define MAX_CLIENTS FD_SETSIZE / 2
struct client_info
{
    int connect_fd; // 负责通信的连接
    int transfer_fd; // 负责文件传输的连接
    int state; // 客户状态，包括：
    // NOT_LOGGED_IN
    // LOGGING_IN
    // LOGGED_IN
    // FILE_TRANSFERING
    int mode; // 文件传输模式
    struct sockaddr_in addr;
    int rw;
    char filename[200];
};
//extern int client_conn_fds[MAX_CLIENTS], client_state[MAX_CLIENTS];
//extern int client_trans_fds[MAX_CLIENTS];
extern struct client_info clients[MAX_CLIENTS];
extern int max_i, max_fd;
extern fd_set allset;

int manage_conn_fds(int cur_fd)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].connect_fd < 0)
        {
            clients[i].connect_fd = cur_fd;
            break;
        }
    }
    if (i == MAX_CLIENTS)
    {
        printf("Error too many clients(): %s(%d)\n", strerror(errno), errno);
        return 0;
    }
    FD_SET(cur_fd, &allset);
    if (cur_fd > max_fd)
    {
        max_fd = cur_fd;
    }
    if (i > max_i)
    {
        max_i = i;
    }
    return 1;
}

void close_conn_fd(int i)
{
    int fd = clients[i].connect_fd;
    if (fd != -1) 
    {
        close(fd);
        clients[i].connect_fd = -1;
        clients[i].state = NOT_LOGGED_IN;
        clients[i].mode = NO_CONNECTION;
        FD_CLR(fd, &allset);
    }
   
}

void close_trans_fd(int i)
{
    int fd = clients[i].transfer_fd;
    if (fd != -1)
    {
        close(fd);
        clients[i].transfer_fd = -1;
        clients[i].state = LOGGED_IN;
        clients[i].mode = NO_CONNECTION;
        FD_CLR(fd, &allset);
    }
    
}

void manage_trans_fds(int cur_fd, int i)
{
    clients[i].transfer_fd = cur_fd;
    FD_SET(cur_fd, &allset);
    if (cur_fd > max_fd)
    {
        max_fd = cur_fd;
    }
}