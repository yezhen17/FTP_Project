#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>    
#include <sys/stat.h>

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
#include <fcntl.h>

#include "rw.h"
#include "fd_manager.h"

#define NOT_LOGGED_IN 0
#define LOGGING_IN 1
#define LOGGED_IN 2
#define FILE_TRANSFERING 3
#define NO_CONNECTION 0
#define PASV_MODE 1
#define PORT_MODE 2
#define LISTENING 3
#define READY_TO_CONNECT 3
#define READ 0
#define WRITE 1
#define BUFSIZE 8192
#define MAX_CLIENTS FD_SETSIZE / 2



int client_conn_fds[MAX_CLIENTS], client_state[MAX_CLIENTS];
int client_trans_fds[MAX_CLIENTS];
struct ip_and_port connection_infos[MAX_CLIENTS];
int max_i, max_fd;
fd_set allset;

int start_listening(int port) {
    int listenfd;
    struct sockaddr_in addr;
    //创建socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    //设置本机的ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //监听"0.0.0.0"

    //将本机的ip和port与socket绑定
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    //开始监听socket
    if (listen(listenfd, 10) == -1)
    {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    return listenfd;
}

int start_connecting(int port, char *ip) {
    int sockfd;
    struct sockaddr_in addr;

    //创建socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    //设置目标主机的ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    printf("nmd,%s\n", ip);
    printf("%d\n", port);
    if (inet_pton(AF_INET, "192.168.11.131", &addr.sin_addr) <= 0)
    { //转换ip地址:点分十进制-->二进制
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
    printf("this is sockfd: %d\n",sockfd);
    //连接上目标主机（将socket和目标主机连接）-- 阻塞函数
    sleep(3);
    printf("waking up.\n");
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int check_port_used(int port) {
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




int PORT_param(char *src) {
    int p = 0;
    int comma_count = 0;
    while(src[p] != '\0') {
        if(src[p] == ',') {
            comma_count++;
            if(comma_count == 4) {
                break;
            } 
            src[p] = '.';
        }
        p++;
    }
    //strncpy(dest, src, p);
    src[p] = '\0';
    p++;
    int fst_hlf = 0;
    int scd_hlf = 0;
    while(src[p] != '\0') {
        if(src[p] == ',') {
            break;
        } else if('0' > src[p] || '9' < src[p]) {
            return -1;
        } else {
            fst_hlf *= 10;
            fst_hlf += src[p] - '0';
        }
        p++;
    }
    p++;
    while(src[p] != '\0') {
        if('0' > src[p] || '9' < src[p]) {
            return -1;
        } else {
            scd_hlf *= 10;
            scd_hlf += src[p] - '0';
        }
        p++;
    }
    return (fst_hlf << 8) + scd_hlf;
}

int IsSocketClosed(int fd)
{
    char buff[32];
    int recvBytes = recv(fd, buff, sizeof(buff), MSG_PEEK);

    int sockErr = errno;

    //cout << "In close function, recv " << recvBytes << " bytes, err " << sockErr << endl;

    if (recvBytes > 0) //Get data
        return 0;

    if ((recvBytes == -1) && (sockErr == EWOULDBLOCK)) //No receive data
        return 0;

    return 1;
}



int main(int argc, char **argv) {
    srand((int)time(0));
    int listenfd, connfd, tmpfd;		//监听socket和连接socket不一样，后者用于数据传输
    int i;
    int nready;
    
    
    struct sockaddr_in addr;
    fd_set rset, wset;
    char sentence[8192];
    int p;
    int len;

    listenfd = start_listening(6789);
    if(listenfd == -1) {
        return 0;
    }

    int server_state = NOT_LOGGED_IN;

    max_fd = listenfd;
    max_i = -1;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_conn_fds[i] = -1;
        client_trans_fds[i] = -1;
        client_state[i] = NOT_LOGGED_IN;
        connection_infos[i].mode = NO_CONNECTION;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    //持续监听连接请求
    while (1) {
        
        rset = allset;
        wset = allset;
        nready = select(max_fd + 1, &rset, &wset, NULL, NULL);
        if (nready == -1) {
            if (errno == EINTR) {
                continue; // to be confirmed
            } else {
                printf("Error select(): %s(%d)\n", strerror(errno), errno);
                return 1;
            }
        }

        if (FD_ISSET(listenfd, &rset)) {
            
            //等待client的连接 -- 阻塞函数
            if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
                printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            }
            else if (!manage_conn_fds(connfd)) {
                close(connfd);
            }
            
            if (--nready <= 0) {
                continue;
            }
        }
        for (i = 0; i <= max_i; i++) {
            if ((tmpfd = client_conn_fds[i]) < 0)
            {
                continue;
            }
            if (FD_ISSET(tmpfd, &rset)) {
                
                if (IsSocketClosed(tmpfd)) {
                    close_conn_fd(tmpfd, i);
                    continue;
                }

                int login_state = client_state[i];
                char message[1024];
                get_cmd(tmpfd, message, 1023);
                printf("%s\n", message);

                char response[100];
                if (login_state == NOT_LOGGED_IN) {
                    if(strcmp(message, "q") == 0) {
                        strcpy(response, "Permission granted. Please enter an email address as the password:\r\n");
                        login_state = LOGGING_IN;
                    } else {
                        strcpy(response, "Please log in.\r\n");
                    }
                } else if (login_state == LOGGING_IN) {
                    if(strncmp(message, "PASS", 4) == 0) {
                        strcpy(response, "You have successfully logged in. Welcome.\r\n");
                        login_state = LOGGED_IN;
                    } else {
                        strcpy(response, "Please use PASS to pass a password.\r\n");
                    }
                } else if (login_state == LOGGED_IN) {
                    if(strcmp(message, "SYST") == 0) {
                        strcpy(response, "215 UNIX Type: L8\r\n");
                    } 
                    
                    else if (strncmp(message, "TYPE", 4) == 0) {
                        if (strcmp(message, "TYPE I") == 0) {
                            strcpy(response, "200 Type set to I.\r\n");
                        } else {
                            strcpy(response, "Doesn't support setting to this type.\r\n");
                        }
                    }

                    else if (strcmp(message, "QUIT") == 0 || strcmp(message, "ABOR") == 0)
                    {
                        strcpy(response, "221 Goodbye.\r\n");
                        send_response(response, tmpfd);

                        close_trans_fd(client_trans_fds[i], i);
                        close_conn_fd(tmpfd, i);
                        
                        continue;
                    }

                    else if (strncmp(message, "RETR ", 5) == 0) {
                        if(connection_infos[i].mode == NO_CONNECTION) {
                            strcpy(response, "425 Please establish a connection first.\r\n");
                        }
                        if (connection_infos[i].mode == READY_TO_CONNECT) {
                            printf("nmd,%s\n", connection_infos[i].ip);
                            strcpy(response, "testing.\r\n");
                            send_response(response, tmpfd);
                            connection_infos[i].mode = PORT_MODE;
                            connection_infos[i].rw = READ;
                            int transfd = start_connecting(connection_infos[i].port, connection_infos[i].ip);
                            if(transfd == -1) {}
                            else {
                                printf("connected!\n");
                                manage_trans_fds(transfd, i);
                            }
                            continue;
                        }
                        if (connection_infos[i].mode == LISTENING)
                        {
                        }
                    } 
                    
                    else if (strncmp(message, "PORT", 4) == 0) {
                        char port_ip[30];
                        strcpy(port_ip, message + 5);
                        //printf("%s", port_ip);
                        int port_num = PORT_param(port_ip);
                        //printf("%s", port_ip);
                        //printf("%d", port_num);
                        strcpy(connection_infos[i].ip, port_ip);
                        connection_infos[i].port = port_num;
                        connection_infos[i].mode = READY_TO_CONNECT;
                        strcpy(response, "200 PORT command successful.\r\n");
                    } 
                    
                    
                    else if (strcmp(message, "PASV") == 0) {
                        char port_ip[30];
                        strcpy(port_ip, "192.168.11.131");
                        int port_num; 
                        do
                        {
                            port_num = rand() / 45536 + 20000;
                        } while(check_port_used(port_num));
                        
                        int fst_hlf = port_num / 256;
                        int scd_hlf = port_num % 256;
                        port_num = 49999;
                        connection_infos[i].port = port_num;
                        connection_infos[i].mode = LISTENING;

                        int transfd;
                        transfd = start_listening(port_num);
                        if(transfd == -1) {
                            strcpy(response, "Failed to open a new socket.\r\n");
                            continue;
                        }

                        manage_trans_fds(transfd, i);

                        strcpy(response, "49999.\r\n");
                    } 
                }
                else {
                    strcpy(response, "No operations allowed while transferring.");
                }
                //printf("wtf, %s", response);
                send_response(response, tmpfd);
                client_state[i] = login_state;
            }
            if ((tmpfd = client_trans_fds[i]) < 0)
            {
                continue;
            }
            if (FD_ISSET(tmpfd, &wset)) {
                int mode = connection_infos[i].mode;
                if (mode == LISTENING) {
                    int transfd;
                    //等待client的连接 -- 阻塞函数
                    if ((transfd = accept(tmpfd, NULL, NULL)) == -1)
                    {
                        printf("Error accept(): %s(%d)\n", strerror(errno), errno);
                        continue; // ?what
                    }
                    close_trans_fd(tmpfd, i);
                    manage_trans_fds(transfd, i);
                    if (--nready <= 0)
                    {
                        continue;
                    }
                    connection_infos[i].mode = PASV_MODE;
                }
                if (mode == READY_TO_CONNECT) {

                }
                if (mode == PORT_MODE || mode == PASV_MODE)
                {
                    if(connection_infos[i].rw == READ) {
                        //char response[100] = "this is a test, man.";
                        //send_response(response, tmpfd);

                        int f, nbytes;
                        char buffer[BUFSIZE];
                        memset(&buffer, 0, BUFSIZE);
                        char filename[200];
                        //strcpy(filename, connection_infos[i].filename);
                        strcpy(filename, "rw.h");
                        printf("get filename : [ %s ]\n", filename);
                        if ((f = open(filename, O_RDONLY)) < 0) //以只读的方式打开client要下载的文件
                        {
                            printf("Open file Error!\n");
                            buffer[0] = 'N';
                            if (write(tmpfd, buffer, BUFSIZE) < 0)
                            {
                                printf("Write Error!At commd_get 1\n");
                                exit(1);
                            }
                            return;
                        }

                        // buffer[0] = 'Y'; //此处标示出文件读取成功
                        // if (write(tmpfd, buffer, BUFSIZE) < 0)
                        // {
                        //     printf("Write Error! At commd_get 2!\n");
                        //     close(f);
                        //     exit(1);
                        // }

                        while ((nbytes = read(f, buffer, BUFSIZE)) > 0) //将文件内容读到buffer中
                        {
                            printf("a line\n");
                            //printf("%s", buffer);
                            if (write(tmpfd, buffer, nbytes) < 0) //将buffer发送回client
                            {
                                printf("Write Error! At commd_get 3!\n");
                                close(f);
                                exit(1);
                            }
                        }

                        close(f);
                        close_trans_fd(tmpfd, i);

                        continue;
                    }
                    else {

                    }
                }
            }
        }
        
    }

    close(listenfd);
}

