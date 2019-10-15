#include "soc_utils.h"
#include "global.h"

int start_listening(int port)
{
    int fd;
    struct sockaddr_in addr;
    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    if (listen(fd, 10) == -1)
    {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    return fd;
}

int IsSocketClosed(int fd)
{
    char buff[32];
    int recvBytes = recv(fd, buff, sizeof(buff), MSG_PEEK);

    int sockErr = errno;

    if (recvBytes > 0) //Get data
        return 0;

    if ((recvBytes == -1) && (sockErr == EWOULDBLOCK)) //No receive data
        return 0;

    return 1;
}

int start_connecting(int sockfd, struct sockaddr_in addrs)
{
    struct sockaddr_in addr;

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int check_port_used(int port)
{
    int used = 0;
    int tmpfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    if (bind(tmpfd, (struct sockaddr *)(&addr), sizeof(addr)) < 0)
    {
        used = 1;
    }
    close(tmpfd);
    return used;
}