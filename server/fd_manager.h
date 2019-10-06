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
#define MAX_CLIENTS FD_SETSIZE / 2
struct ip_and_port
{
    int mode;
    char ip[20];
    int port;
    int rw;
    char filename[200];
};
extern int client_conn_fds[MAX_CLIENTS], client_state[MAX_CLIENTS];
extern int client_trans_fds[MAX_CLIENTS];
extern struct ip_and_port connection_infos[MAX_CLIENTS];
extern int max_i, max_fd;
extern fd_set allset;

int manage_conn_fds(int cur_fd)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_conn_fds[i] < 0)
        {
            client_conn_fds[i] = cur_fd;
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

void close_conn_fd(int cur_fd, int i)
{
    close(cur_fd);
    client_conn_fds[i] = -1;
    client_state[i] = NOT_LOGGED_IN;
    connection_infos[i].mode = NO_CONNECTION;
    FD_CLR(cur_fd, &allset);
}

void close_trans_fd(int cur_fd, int i)
{
    close(cur_fd);
    client_trans_fds[i] = -1;
    client_state[i] = LOGGED_IN;
    connection_infos[i].mode = NO_CONNECTION;
    FD_CLR(cur_fd, &allset);
}

void manage_trans_fds(int cur_fd, int i)
{
    client_trans_fds[i] = cur_fd;
    FD_SET(cur_fd, &allset);
    if (cur_fd > max_fd)
    {
        max_fd = cur_fd;
    }
}