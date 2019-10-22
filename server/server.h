#ifndef SERVER_H
#define SERVER_H

// prepared for commands from client
int serve_client(int fd, int idx);

// upload file
void upload_file(int idx);

// download file
void download_file(int idx);

// implement LIST command through shell command ls
void reply_list(int idx, char *dest);

#endif