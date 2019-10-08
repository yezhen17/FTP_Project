#ifndef RW_H
#define RW_H
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

void get_cmd(int fd, char *message, int message_len);

void send_response(char *response, int tmpfd);

int PORT_param(char *src);

int strip_crlf(char *sentence, int len);


void send_resp(int fd, int code, char *custom_resp);

#endif