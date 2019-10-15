#include "cmds.h"
#include "global.h"
#include "rw_utils.h"
#include "fd_manager.h"
#include "dir_utils.h"

#define MAX_CLIENTS FD_SETSIZE / 2
struct client_info clients[MAX_CLIENTS];

int cmd_router(char *cmd, char *param, int idx)
{
    int code;
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;

    // 文件传输时无视一切命令
    if (state == FILE_TRANSFERING)
    {
        send_resp(fd, 530, NULL);
        return 530;
    }

    if (strcmp(cmd, "USER") == 0)
    {
        code = cmd_user(param, idx);
    }
    else if (strcmp(cmd, "PASS") == 0)
    {
        code = cmd_pass(param, idx);
    }

    // 未登录状态无视除了USER PASS 以外的命令
    else if (state == NOT_LOGGED_IN || state == LOGGING_IN)
    {
        send_resp(fd, 530, NULL);
        return 530;
    }
    if (strcmp(cmd, "RNTO") == 0)
    {
        code = cmd_rnto(param, idx);
    }
    else if (state == WAITING_RNTO) {
        send_resp(fd, 530, NULL);
        return 530;
    }
    if (strcmp(cmd, "RNFR") == 0)
    {
        code = cmd_rnfr(param, idx);
    }

    else if (strcmp(cmd, "TYPE") == 0)
    {
        code = cmd_type(param, idx);
    }
    else if (strcmp(cmd, "SYST") == 0)
    {
        code = cmd_syst(param, idx);
    }
    else if (strcmp(cmd, "QUIT") == 0 || strcmp(cmd, "ABOR") == 0)
    {
        code = cmd_quit(param, idx);
    }
    else if (strcmp(cmd, "PORT") == 0)
    {
        code = cmd_port(param, idx);
    }
    else if (strcmp(cmd, "PASV") == 0)
    {
        code = cmd_pasv(param, idx);
    }
    else if (strcmp(cmd, "STOR") == 0)
    {
        code = cmd_stor(param, idx);
    }
    else if (strcmp(cmd, "RETR") == 0)
    {
        code = cmd_retr(param, idx);
    }
    else if (strcmp(cmd, "PWD") == 0)
    {
        code = cmd_pwd(param, idx);
    }
    else if (strcmp(cmd, "CWD") == 0)
    {
        code = cmd_cwd(param, idx);
    }
    else if (strcmp(cmd, "MKD") == 0)
    {
        code = cmd_mkd(param, idx);
    }
    else if (strcmp(cmd, "RMD") == 0)
    {
        code = cmd_rmd(param, idx);
    }
    else if (strcmp(cmd, "LIST") == 0)
    {
        code = cmd_list(param, idx);
    }
    else if (strcmp(cmd, "REST") == 0)
    {
        code = cmd_rest(param, idx);
    }
    return code;
}

int cmd_user(char *param, int idx)
{
    printf("%s\n", param);
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;
    if (param == NULL)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }
    if (state != NOT_LOGGED_IN)
    {
        send_resp(fd, 530, NULL);
        return 530;
    }
    if (strcmp(param, "anonymous") == 0)
    {
        clients[idx].state = LOGGING_IN;
        send_resp(fd, 331, NULL);
        return 331;
    }
    else
    {
        send_resp(fd, 530, NULL);
        return 530;
    }
}

int cmd_pass(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;
    if (state != LOGGING_IN)
    {
        send_resp(fd, 503, NULL);
        return 503;
    }
    clients[idx].state = LOGGED_IN;
    send_resp(fd, 230, NULL);
    return 230;
}

int cmd_syst(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }
    send_resp(fd, 215, NULL);
    return 215;
}

int cmd_type(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;
    if (strcmp(param, "I") == 0)
    {
        send_resp(fd, 200, "Type set to I.");
        return 200;
    }
    if (strcmp(param, "A") == 0)
    {
        send_resp(fd, 200, "Type set to I.");
        return 200;
    }
    else
    {
        send_resp(fd, 504, NULL);
        return 504;
    }
}

int cmd_quit(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }

    send_resp(fd, 221, NULL);
    close_trans_fd(idx);
    close_conn_fd(idx);
    return 221;
}

int cmd_pasv(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int trans_fd = clients[idx].transfer_fd;
    if (param != NULL)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }
    // 已有文件传输连接
    if (trans_fd != -1)
    {
        close_trans_fd(idx);
    }

    int port;
    do
    {
        port = rand() % 45536 + 20000;
    } while (check_port_used(port));

    int h1, h2, h3, h4, p1, p2;
    sscanf("166.111.82.233", "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
    p1 = port / 256;
    p2 = port % 256;
    char resp[30];
    sprintf(resp, "=%d,%d,%d,%d,%d,%d", h1, h2, h3, h4, p1, p2);

    clients[idx].mode = LISTENING;

    trans_fd = start_listening(port);
    if (trans_fd == -1)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }

    manage_trans_fds(trans_fd, idx);
    send_resp(fd, 227, resp);
    return 227;
}

int cmd_port(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int trans_fd = clients[idx].transfer_fd;

    int h1, h2, h3, h4, p1, p2;
    int count;
    count = sscanf(param, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
    if (count != 6)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }
    if (h1 < 0 || h2 < 0 || h3 < 0 || h4 < 0 || p1 < 0 || p2 < 0 ||
        h1 > 255 || h2 > 255 || h3 > 255 || h4 > 255 || p1 > 255 || p2 > 255)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }

    // 已有文件传输连接
    if (trans_fd != -1)
    {
        close_trans_fd(idx);
    }

    char ipaddr[30];

    sprintf(ipaddr, "%d.%d.%d.%d", h1, h2, h3, h4);
    clients[idx].mode = READY_TO_CONNECT;
    //设置客户端发送的IP
    memset(&(clients[idx].addr), 0, sizeof(clients[idx].addr));
    clients[idx].addr.sin_family = AF_INET;
    clients[idx].addr.sin_port = htons((p1 << 8) + p2);

    //转换ip地址:点分十进制-->二进制
    if (inet_pton(AF_INET, ipaddr, &(clients[idx].addr.sin_addr)) <= 0)
    {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        send_resp(fd, 501, NULL);
        return 500;
    }

    //创建socket
    if ((trans_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        send_resp(fd, 500, NULL);
        return 500;
    }
    manage_trans_fds(trans_fd, idx);

    send_resp(fd, 200, "PORT command successful.");
    return 200;
}

int prepare_transfer(char *param, int idx) {
    int fd = clients[idx].connect_fd;
    int mode = clients[idx].mode;
    printf("transfering");
    send_resp(fd, 150, NULL);
    if (mode == NO_CONNECTION) // 有待考虑
    {
        send_resp(fd, 425, NULL);
        return 425;
    }
    if (mode == READY_TO_CONNECT)
    {
        //strcpy(response, "testing.\r\n");
        //send_response(response, tmpfd);
        clients[idx].mode = TRANSFER_READY;
        
        int trans_fd = clients[idx].transfer_fd;
        struct sockaddr_in addr = clients[idx].addr;
        if (connect(trans_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            printf("Error connect(): %s(%d)\n", strerror(errno), errno);
            close(trans_fd);
            send_resp(fd, 425, NULL);
            return 425;
        }
        //int transfd = start_connecting(clients[i].addr);
        //if(transfd == -1) {}
        //else {
        //    printf("connected!\n");
        //    manage_trans_fds(transfd, i);
        //}
        printf("starting man!\n");
        strcpy(clients[idx].filename, param);
    }
    if (mode == LISTENING)
    {
        printf("set done\n");
        clients[idx].mode = PASV_MODE;
        strcpy(clients[idx].filename, param);
    }
}

int cmd_retr(char *param, int idx)
{
    prepare_transfer(param, idx);
    clients[idx].rw = READ;
}

int cmd_stor(char *param, int idx)
{
    prepare_transfer(param, idx);
    clients[idx].rw = WRITE;
}

int cmd_rest(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }
    int start_pos = -1;
    if(sscanf(param, "%d", &start_pos) == 1 && start_pos >= 0)
    {
        send_resp(fd, 350, "REST set");
        return 350;
    }
    else
    {
        send_resp(fd, 501, NULL);
        return 501;
    }
    
    clients[idx].start_pos = start_pos;
    send_resp(clients[idx].connect_fd, 350, NULL);

}

int cmd_mkd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return 504;
    }

    if (strstr(param, "..") != NULL)
    {
        //不能有..
        send_resp(fd, 530, NULL);
        return 530;
    }
    char dest[200];
    gen_absdir(clients[idx].prefix, param, dest);
    printf("%s", dest);
    if (mkdir(dest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) {
        send_resp(fd, 250, NULL);
        return 250;
    }
    else {
        send_resp(fd, 550, NULL);
        return 550;
    }
}

int cmd_cwd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return 504;
    }

    if (strstr(param, "..") != NULL)
    {
        //不能有..
        send_resp(fd, 530, NULL);
        return 530;
    }

    int code;
    char dest[200];
    gen_absdir(clients[idx].prefix, param, dest);
    if (folder_isvalid(dest))
    {
        strcpy(clients[idx].prefix, dest);
        send_resp(fd, 250, NULL);
        return 250;
    }
    else
    {
        send_resp(fd, 550, NULL);
        return 550;
    }
}

int cmd_pwd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        send_resp(fd, 501, NULL);
        return 501;
    }
    char resp[200];
    // may need to replace double quotes later
    sprintf(resp, "\"%s\" is current directory.", clients[idx].prefix);
    send_resp(fd, 257, resp);
    return 257;
}

int cmd_list(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        char dest[256];
        gen_absdir(clients[idx].prefix, param, dest);
        if (!file_isvalid(dest))
        {
            send_resp(fd, 451, "failed, uncorrect pathname.");
            //sess->current_pasv = -1;
            return 451;
        }
        else
        {
            //send_resp(fd, 150, "Begin listing.");
            // int temp_code = reply_list(idx, dest);
            // send_resp(fd, temp_code, NULL);
            // //sess->current_pasv = -1;
            // return temp_code;
            prepare_transfer(dest, idx);
            clients[idx].rw = LIST;
        }
    }
    else
    {
        //send_resp(fd, 150, "Begin listing.");
        // int temp_code = reply_list(idx, clients[idx].prefix);
        // send_resp(fd, temp_code, NULL);
        // //sess->current_pasv = -1;
        // return temp_code;
        prepare_transfer(clients[idx].prefix, idx);
        clients[idx].rw = LIST;
    }
}

int cmd_rmd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return 504;
    }

    if (strstr(param, "..") != NULL)
    {
        //不能有..
        send_resp(fd, 530, NULL);
        return 530;
    }
    char dest[200];
    gen_absdir(clients[idx].prefix, param, dest);
    if (recursive_rmdir(dest) == 0)
    {
        send_resp(fd, 250, NULL);
        return 250;
    }
    else
    {
        send_resp(fd, 550, NULL);
        return 550;
    }
}

int cmd_rnfr(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return 504;
    }

    if (strstr(param, "..") != NULL)
    {
        //不能有..
        send_resp(fd, 530, NULL);
        return 530;
    }
    char dest[200];
    gen_absdir(clients[idx].prefix, param, dest);
    if (file_isvalid(dest))
    {
        clients[idx].state = WAITING_RNTO;
        strcpy(clients[idx].rename_file, dest);
        send_resp(fd, 350, NULL);
        return 350;
    }
    else
    {
        send_resp(fd, 550, NULL);
        return 550;
    }
}

int cmd_rnto(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;
    clients[idx].state = LOGGED_IN; // 无论成功与否都结束RENAME？
    if (state != WAITING_RNTO) 
    {
        send_resp(fd, 503, "Previous command is not RNFR.");
        return 503;
    }
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return 504;
    }

    if (strstr(param, "..") != NULL)
    {
        //不能有..
        send_resp(fd, 530, NULL);
        return 530;
    }
    char dest[200];
    gen_absdir(clients[idx].prefix, param, dest);

    char shell_cmd[512];
    sprintf(shell_cmd, "mv \"%s\" \"%s\"", clients[idx].rename_file, dest);
    if (system(shell_cmd) == -1)
    {
        printf("Error running shell: %s(%d)\n", strerror(errno), errno);
        send_resp(fd, 550, NULL);
        return 550;
    }
    else
    {
        send_resp(fd, 250, NULL);
        return 250;
    }
}