#include "fd_manager.h"
#include "global.h"

extern struct client_info clients[MAX_CLIENTS];
extern int max_i, max_fd;
extern fd_set allset;
extern char root_folder[256];

void init_fd_manager(int fd)
{
    max_fd = fd;
    max_i = -1;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].connect_fd = -1;
        clients[i].transfer_fd = -1;
        clients[i].state = NOT_LOGGED_IN;
        clients[i].mode = NO_CONNECTION;
        clients[i].start_pos = 0;
        strcpy(clients[i].prefix, root_folder);
    }
    FD_ZERO(&allset);
    FD_SET(fd, &allset);
}

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
        clients[i].start_pos = 0;
        strcpy(clients[i].prefix, root_folder); 
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
        clients[i].start_pos = 0;
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