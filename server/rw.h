#include <sys/socket.h>
#include <sys/select.h>

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

void get_cmd(int fd, char *message, int message_len)
{
    int p = 0;
    while (1)
    {
        int n = read(fd, message + p, message_len - p);
        if (n < 0)
        {
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            continue;
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            p += n;
            if (message[p - 1] == '\n')
            {
                break;
            }
        }
    }
    message[p - 1] = '\0';
}

void send_response(char *response, int tmpfd)
{
    int len = strlen(response);
    response[len] = '\0';
    //printf("%d\n", len);
    int p = 0;
    while (p < len)
    {
        int n = write(tmpfd, response + p, len + 1 - p);
        if (n < 0)
        {
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            continue;
        }
        else
        {
            p += n;
        }
    }
}