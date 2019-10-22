/*
socket related utils
*/

#ifndef SOC_UTILS_H
#define SOC_UTILS_H

#include <netinet/in.h>

// check whether a port is used
int check_port_used(int port);

// create a socket and listen on it, returns the socket fd if success, otherwise -1
int start_listening(int port);

// starts a connection, returns -1 if fail
int start_connecting(int sockfd, struct sockaddr_in addrs);

// check whether a socket is closed
int is_socket_closed(int fd);

// 获取本机ip，参考了https://blog.csdn.net/yldfree/article/details/83825945
int get_local_ip(char *ipbuf);

#endif