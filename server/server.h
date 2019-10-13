#ifndef SERVER_H
#define SERVER_H
#include <sys/select.h>

extern int listen_port;

extern int max_i, max_fd;
extern fd_set allset;
int start_listening(int port);

int start_connecting(int sockfd, struct sockaddr_in addrs);
int IsSocketClosed(int fd);

int serve_client(int fd, int idx);
#endif