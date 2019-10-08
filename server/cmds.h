#ifndef CMDS_H
#define CMDS_H

#include "rw.h"
#include "fd_manager.h"
#include "utils.h"

#define NOT_LOGGED_IN 0
#define LOGGING_IN 1
#define LOGGED_IN 2
#define FILE_TRANSFERING 3
#define NO_CONNECTION 0
#define PASV_MODE 1
#define PORT_MODE 2
#define LISTENING 3
#define READY_TO_CONNECT 4
#define READ 0
#define WRITE 1
#define BUFSIZE 8192
#define MAX_CLIENTS FD_SETSIZE / 2

#define MAX_CLIENTS FD_SETSIZE / 2
struct client_info clients[MAX_CLIENTS];


int cmd_router(char *cmd, char *param, int idx);

int cmd_user(char *param, int idx);

int cmd_pass(char *param, int idx);

int cmd_syst(char *param, int idx);

int cmd_type(char *param, int idx);

int cmd_quit(char *param, int idx);

int cmd_pasv(char *param, int idx);

int cmd_port(char *param, int idx);

int cmd_retr(char *param, int idx);

int cmd_stor(char *param, int idx);

int cmd_mkd(char *param);

int cmd_cwd(char *param);

int cmd_pwd(char *param);

int cmd_list(char *param);

int cmd_rmd(char *param);

int cmd_rnfr(char *param);

int cmd_rnto(char *param);

#endif