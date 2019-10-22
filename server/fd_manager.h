/*
manages fds to implement multi-client
*/
#ifndef FD_MANAGER_H
#define FD_MANAGER_H

// inits this manager
void init_fd_manager(int fd);

// adds a connection fd (new client)
int manage_conn_fds(int cur_fd);

// closes connection fd
void close_conn_fd(int i);

// closes transfer fd
void close_trans_fd(int i);

// adds a transfer fd
void manage_trans_fds(int cur_fd, int i);

#endif