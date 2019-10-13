#ifndef UTIL_H
#define UTIL_H
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

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
#include <fcntl.h>
#include <getopt.h>

int check_port_used(int port);

void gen_absdir(char *prefix, char *dir, char *dest);
int file_isvalid(char *pth);
int folder_isvalid(char *pth);

int recursive_rmdir(char *pth);
#endif