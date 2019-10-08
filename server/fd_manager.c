#include "fd_manager.h"
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