extern char server_ip[20];
extern int server_port;

#define NOT_LOGGED_IN 0
#define LOGGING_IN 1
#define LOGGED_IN 2
#define FILE_TRANSFERING 3



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