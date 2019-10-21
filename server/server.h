#ifndef SERVER_H
#define SERVER_H

int serve_client(int fd, int idx);

void upload_file(int idx);

void download_file(int idx);

#endif