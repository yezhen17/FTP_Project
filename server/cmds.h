#ifndef CMDS_H
#define CMDS_H

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

int cmd_rest(char *param, int idx);

int cmd_mkd(char *param, int idx);

int cmd_cwd(char *param, int idx);

int cmd_pwd(char *param, int idx);

int cmd_list(char *param, int idx);

int cmd_rmd(char *param, int idx);

int cmd_rnfr(char *param, int idx);

int cmd_rnto(char *param, int idx);

#endif