#include "soc_utils.h"
#include "global.h"

int start_listening(int port)
{
    int fd;
    struct sockaddr_in addr;
    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        //printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        //printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    if (listen(fd, 10) == -1)
    {
        //printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    return fd;
}

int is_socket_closed(int fd)
{
    char buff[32];
    int recvBytes = recv(fd, buff, sizeof(buff), MSG_PEEK);

    int sockErr = errno;

    if (recvBytes > 0)
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
        //printf("Error connect(): %s(%d)\n", strerror(errno), errno);
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

int get_local_ip(char *ipbuf)
{
    int i = 0;
    int sockfd;
    struct ifconf ifc;
    char buf[1024] = {0};
    struct ifreq *ifr;

    ifc.ifc_len = 1024;
    ifc.ifc_buf = buf;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        //printf("socket error\n");
        return -1;
    }
    ioctl(sockfd, SIOCGIFCONF, &ifc);
    ifr = (struct ifreq *)buf;

    for (i = (ifc.ifc_len / sizeof(struct ifreq)); i > 0; i--)
    {
        // printf("net name: %s\n", ifr->ifr_name);
        inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr, ipbuf, 20);
        // printf("ip: %s \n", ipbuf);
        ifr = ifr + 1;
    }
    return 0;
}