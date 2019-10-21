#ifndef SOC_UTILS_H
#define SOC_UTILS_H

// socket utils

#include <netinet/in.h>

int check_port_used(int port);

int start_listening(int port);

int start_connecting(int sockfd, struct sockaddr_in addrs);

int IsSocketClosed(int fd);

int get_local_ip(char *ipbuf);

#endif