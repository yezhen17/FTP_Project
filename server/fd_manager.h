#ifndef FD_MANAGER_H
#define FD_MANAGER_H

// manages fds to implement multi-client

void init_fd_manager(int fd);

int manage_conn_fds(int cur_fd);

void close_conn_fd(int i);

void close_trans_fd(int i);

void manage_trans_fds(int cur_fd, int i);

#endif