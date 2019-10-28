/*
all commands, and the command router
*/

#ifndef CMDS_H
#define CMDS_H

void cmd_router(char *cmd, char *param, int idx);

void cmd_user(char *param, int idx);

void cmd_pass(char *param, int idx);

void cmd_syst(char *param, int idx);

void cmd_type(char *param, int idx);

void cmd_quit(char *param, int idx);

void cmd_abor(char *param, int idx);

void cmd_pasv(char *param, int idx);

void cmd_port(char *param, int idx);

void cmd_retr(char *param, int idx);

void cmd_stor(char *param, int idx);

void cmd_rest(char *param, int idx);

void cmd_mkd(char *param, int idx);

void cmd_cwd(char *param, int idx);

void cmd_pwd(char *param, int idx);

void cmd_list(char *param, int idx);

void cmd_rmd(char *param, int idx);

void cmd_dele(char *param, int idx);

void cmd_rnfr(char *param, int idx);

void cmd_rnto(char *param, int idx);

void prepare_transfer(char *param, int idx);

#endif