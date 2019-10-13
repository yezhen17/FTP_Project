#include <sys/socket.h>

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
#include <getopt.h>

#include "cmds.h"
#include "server.h"

int start_listening(int port) {
    int listenfd;
    struct sockaddr_in addr;
    //����socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    //���ñ�����ip��port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //����"0.0.0.0"

    //��������ip��port��socket��
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    //��ʼ����socket
    if (listen(listenfd, 10) == -1)
    {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }

    return listenfd;
}

int start_connecting(int sockfd, struct sockaddr_in addrs) {
    struct sockaddr_in addr;

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
    return sockfd;
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

int serve_client(int fd, int idx) {
    
    char sentence[1024];
    char check_s;
    int len;
    char cmd[10];
    char param[100];
    int code;
    //get_cmd(fd, message, 1023);
    //printf("%s\n", message);
    len = recv(fd, sentence, 8192, 0);
    //长度小于2代表断开连接
    if (len <= 0)
        return 0;

    //对消息进行预处理
    sentence[len] = '\0';
    len = strip_crlf(sentence, len);

    //对内容进行匹配
    int word_count = sscanf(sentence, "%s %c", cmd, &check_s);
    printf("%s\n", cmd);
    if (word_count <= 0)
    {
        printf("None string read.\n");
        send_resp(fd, 500, NULL);
    }

    else if (word_count == 1)
    {
        //处理无参数指令
        code = cmd_router(cmd, NULL, idx);
    }
    else
    {
        //处理有参数指令
        strcpy(param, sentence + strlen(cmd) + 1);
        code = cmd_router(cmd, param, idx);
    }

    //code为221则断开连接
    if (code == 221)
        return 0;
    return 1;
}

short unsigned lis_port = 21;
char check_s;

int main(int argc, char **argv) {
    
    srand((int)time(0));
    int listenfd, connfd, tmpfd;		//����socket������socket��һ���������������ݴ���
    int i;
    int nready;
    char root_folder[200];
    strcpy(root_folder, "/home/cyz/tmp");
    int opt;

    const struct option arg_options[] = {

        {"port", required_argument, NULL, 'p'},

        {"root", required_argument, NULL, 'r'},

        {NULL, 0, NULL, 0},
    };

    while ((opt = getopt_long_only(argc, (char *const *)argv, "p:r:", arg_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'r':
            if (access(optarg, 0))
            {
                printf("wrong path given: %s.\n", optarg);
                return 1;
            }
            strcpy(root_folder, optarg);
            break;
        case 'p':
            if (sscanf(optarg, "%hd%c", &lis_port, &check_s) != 1)
            {
                printf("wrong port given: %s.\n", optarg);
                return 1;
            }
            break;
        case '?':
            printf("wrong argument.\n");
            return 1;
        }
    }
    printf("%d\n", lis_port);
    printf("%s\n", root_folder);
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
        clients[i].connect_fd = -1;
        clients[i].transfer_fd = -1;
        clients[i].state = NOT_LOGGED_IN;
        clients[i].mode = NO_CONNECTION;
        strcpy(clients[i].prefix, root_folder);
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    //����������������
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
            
            //�ȴ�client������ -- ��������
            if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
                printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            }
            else if (!manage_conn_fds(connfd)) {
                close(connfd);
            }
            send_resp(connfd, 220, NULL);
            //char response[100];
            //strcpy(response, "220 hello\r\n");
            //send_response(response, connfd);
            if (--nready <= 0) {
                continue;
            }
        }
        for (i = 0; i <= max_i; i++) {
            if ((tmpfd = clients[i].connect_fd) < 0)
            {
                continue;
            }
            if (FD_ISSET(tmpfd, &rset)) {
                if(serve_client(tmpfd, i) == 0)
                {
                    close_trans_fd(i);
                    close_conn_fd(i);
                }
                // if (IsSocketClosed(tmpfd)) {
                //     close_conn_fd(i);
                //     continue;
                // }

                // int login_state = clients[i].state;
                // char message[1024];
                // get_cmd(tmpfd, message, 1023);
                // printf("%s\n", message);

                // char response[100];
                // if (login_state == NOT_LOGGED_IN) {
                //     if(strcmp(message, "q") == 0) {
                //         strcpy(response, "200 Permission granted. Please enter an email address as the password:\r\n");
                //         login_state = LOGGING_IN;
                //     } else {
                //         strcpy(response, "200 Please log in.\r\n");
                //     }
                // } else if (login_state == LOGGING_IN) {
                //     if(strncmp(message, "PASS", 4) == 0) {
                //         strcpy(response, "200 You have successfully logged in. Welcome.\r\n");
                //         login_state = LOGGED_IN;
                //     } else {
                //         strcpy(response, "200 Please use PASS to pass a password.\r\n");
                //     }
                // } else if (login_state == LOGGED_IN) {
                //     if(strcmp(message, "SYST") == 0) {
                //         strcpy(response, "215 UNIX Type: L8\r\n");
                //     } 
                    
                //     else if (strncmp(message, "TYPE", 4) == 0) {
                //         if (strcmp(message, "TYPE I") == 0) {
                //             strcpy(response, "200 Type set to I.\r\n");
                //         } else {
                //             strcpy(response, "Doesn't support setting to this type.\r\n");
                //         }
                //     }

                //     else if (strcmp(message, "QUIT") == 0 || strcmp(message, "ABOR") == 0)
                //     {
                //         strcpy(response, "221 Goodbye.\r\n");
                //         send_response(response, tmpfd);

                //         close_trans_fd(i);
                //         close_conn_fd(i);
                        
                //         continue;
                //     }

                //     else if (strncmp(message, "RETR ", 5) == 0) {
                //         if(clients[i].mode == NO_CONNECTION) {
                //             strcpy(response, "425 Please establish a connection first.\r\n");
                //         }
                //         if (clients[i].mode == READY_TO_CONNECT) {
                //             strcpy(response, "testing.\r\n");
                //             send_response(response, tmpfd);
                //             clients[i].mode = PORT_MODE;
                //             clients[i].rw = READ;
                //             //int transfd = start_connecting(clients[i].addr);
                //             //if(transfd == -1) {}
                //             //else {
                //             //    printf("connected!\n");
                //             //    manage_trans_fds(transfd, i);
                //             //}
                //             continue;
                //         }
                //         if (clients[i].mode == LISTENING)
                //         {
                //         }
                //     } 
                    
                //     else if (strncmp(message, "PORT", 4) == 0) {
                //         char port_ip[30];
                //         strcpy(port_ip, message + 5);
                //         //printf("%s", port_ip);
                //         int port_num = PORT_param(port_ip);
                //         //printf("%s", port_ip);
                //         //printf("%d", port_num);
                //         //strcpy(clients[i].ipaddr, port_ip);
                //         //clients[i].port = port_num;
                //         //clients[i].mode = READY_TO_CONNECT;
                //         strcpy(response, "200 PORT command successful.\r\n");
                //     } 
                    
                    
                //     else if (strcmp(message, "PASV") == 0) {
                //         char port_ip[30];
                //         strcpy(port_ip, "192.168.11.131");
                //         int port_num; 
                //         do
                //         {
                //             port_num = rand() / 45536 + 20000;
                //         } while(check_port_used(port_num));
                        
                //         int fst_hlf = port_num / 256;
                //         int scd_hlf = port_num % 256;
                //         port_num = 49999;
                //         //clients[i].port = port_num;
                //         clients[i].mode = LISTENING;

                //         int transfd;
                //         transfd = start_listening(port_num);
                //         if(transfd == -1) {
                //             strcpy(response, "Failed to open a new socket.\r\n");
                //             continue;
                //         }

                //         manage_trans_fds(transfd, i);

                //         strcpy(response, "49999.\r\n");
                //     } 
                // }
                // else {
                //     strcpy(response, "No operations allowed while transferring.");
                // }
                // //printf("wtf, %s", response);
                // send_response(response, tmpfd);
                // clients[i].state = login_state;
            }
            if ((tmpfd = clients[i].transfer_fd) < 0)
            {
                continue;
            }
            if (FD_ISSET(tmpfd, &rset)){
                int mode = clients[i].mode;
                if (mode == PASV_MODE)
                {
                    int transfd;
                    //�ȴ�client������ -- ��������
                    if ((transfd = accept(tmpfd, NULL, NULL)) == -1)
                    {
                        printf("Error accept(): %s(%d)\n", strerror(errno), errno);
                        continue; // ?what
                    }
                    close_trans_fd(i);
                    manage_trans_fds(transfd, i);
                    clients[i].mode = PORT_MODE;
                    //clients[i].mode = PASV_MODE;
                    printf("listening pass\n");
                }
                if (mode == PORT_MODE || mode == PORT_MODE)
                {
                    if (clients[i].rw == WRITE)
                    {
                        int f, nbytes;
                        char buffer[BUFSIZE];
                        memset(&buffer, 0, BUFSIZE);
                        char filename[200];
                        //strcpy(filename, connection_infos[i].filename);
                        strcpy(filename, "rw.h");
                        printf("get filename : [ %s ]\n", clients[i].filename);
                        if ((f = open(clients[i].filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) //��ֻ���ķ�ʽ��clientҪ���ص��ļ�
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

                        while ((nbytes = read(tmpfd, buffer, BUFSIZE)) > 0) //���ļ����ݶ���buffer��
                        {
                            printf("a line\n");
                            printf("%s", buffer);
                            if (write(f, buffer, nbytes) < 0) //��buffer���ͻ�client
                            {
                                printf("Write Error! At commd_get 3!\n");
                                close(f);
                                exit(1);
                            }
                        }
                        close(f);
                        close_trans_fd(i);
                        send_resp(clients[i].connect_fd, 226, NULL);
                        continue;
                    }
                }
            }
            else if (FD_ISSET(tmpfd, &wset)) {
                
                int mode = clients[i].mode;
                
                if (mode == READY_TO_CONNECT) {

                }
                if (mode == PORT_MODE || mode == PORT_MODE)
                {
                    if(clients[i].rw == READ) {
                        //char response[100] = "this is a test, man.";
                        //send_response(response, tmpfd);

                        int f, nbytes;
                        char buffer[BUFSIZE];
                        memset(&buffer, 0, BUFSIZE);
                        char filename[200];
                        //strcpy(filename, connection_infos[i].filename);
                        strcpy(filename, "rw.h");
                        printf("get filename : [ %s ]\n", clients[i].filename);
                        if ((f = open(clients[i].filename, O_RDONLY)) < 0) //��ֻ���ķ�ʽ��clientҪ���ص��ļ�
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

                        while ((nbytes = read(f, buffer, BUFSIZE)) > 0) //���ļ����ݶ���buffer��
                        {
                            printf("a line\n");
                            printf("%s", buffer);
                            int p = 0;
                            while (p < nbytes)
                            {
                                int n = write(tmpfd, buffer + p, nbytes);
                                if (n < 0)
                                {
                                    printf("Error write(): %s(%d)\n", strerror(errno), errno);
                                    close(connfd);
                                    continue;
                                }
                                else if (n == 0)
                                {
                                    break;
                                }
                                else
                                {
                                    p += n;
                                }
                            }
                            // if (write(tmpfd, buffer, nbytes) < 0) //��buffer���ͻ�client
                            // {
                            //     printf("Write Error! At commd_get 3!\n");
                            //     close(f);
                            //     exit(1);
                            // }
                        }
                        close(f);
                        close_trans_fd(i);
                        send_resp(clients[i].connect_fd, 226, NULL);
                        continue;
                    }
                    else if (clients[i].rw == LIST) {
                        reply_list(i, clients[i].filename);
                    }
                }
            }
        }
        
    }

    close(listenfd);
}

