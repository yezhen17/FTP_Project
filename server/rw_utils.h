#ifndef RW_H
#define RW_H

// read/write utils

void get_cmd(int fd, char *message, int message_len);

//void send_response(char *response, int tmpfd);

// int PORT_param(char *src);

int strip_crlf(char *sentence, int len);

void send_resp(int fd, int code, char *custom_resp);

int safe_send(int fd, char *buf, int len);

int safe_recv(int fd, char *buf, int len);

#endif