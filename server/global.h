/*
连接信息定义的结构体，一些全局变量等等
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <getopt.h>
#include <dirent.h>

#define NOT_LOGGED_IN 0
#define LOGGING_IN 1
#define LOGGED_IN 2
#define FILE_TRANSFERING 3
#define WAITING_RNTO 4

#define NO_CONNECTION 0
#define PASV_MODE 1
#define TRANSFER_READY 2
#define LISTENING 3
#define READY_TO_CONNECT 4

#define READ 0
#define WRITE 1
#define LIST 2

#define BUFSIZE 8192
#define MAX_CLIENTS FD_SETSIZE / 2

struct client_info
{
    int connect_fd;  // fd for control 
    int transfer_fd; // fd for file transfer
    int state;       // state(logged in/not logged in/file transfering, etc.)
    int mode; // file transfer mode, including preparing stages
    struct sockaddr_in addr;
    int rw; // read or write or list
    char filename[256]; // file to retrieve or store
    char prefix[256]; 
    char rename_file[256]; // file new name
    int start_pos; // transfer offset
};

int listen_port;
char root_folder[256];
char local_ip[20];
int max_i, max_fd;
fd_set allset;
struct client_info clients[MAX_CLIENTS];

#endif