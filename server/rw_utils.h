/*
send/receive related utils
*/

#ifndef RW_UTILS_H
#define RW_UTILS_H

// receives and parses a command
void get_cmd(int fd, char *message, int message_len);

// strip the trailing CRLF
int strip_crlf(char *sentence, int len);

// send statistics when quit
void send_statistics(int fd, int code, char *str1, char *str2, char *str3);

// send a response according to code and response text
void send_resp(int fd, int code, char *custom_resp);

// safe sending which ensures correct transmission and warns if not
int safe_send(int fd, char *buf, int len);

// safe receiving which ensures correct transmission and warns if not
int safe_recv(int fd, char *buf, int len);

#endif