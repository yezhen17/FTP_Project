#include "cmds.h"
#include "global.h"
#include "rw_utils.h"
#include "fd_manager.h"
#include "dir_utils.h"
#include "soc_utils.h"

#define MAX_CLIENTS FD_SETSIZE / 2
struct client_info clients[MAX_CLIENTS];

void cmd_router(char *cmd, char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;

    if (strcmp(cmd, "ABOR") == 0)
    {
        cmd_abor(param, idx);
        return;
    }
    // ignore all cmds except ABOR when transfering file
    if (state == FILE_TRANSFERING)
    {
        send_resp(fd, 530, NULL);
        return;
    }

    if (strcmp(cmd, "USER") == 0)
    {
        cmd_user(param, idx);
    }
    else if (strcmp(cmd, "PASS") == 0)
    {
        cmd_pass(param, idx);
    }

    // ignore any cmds other than USER PASS if not logged in
    else if (state == NOT_LOGGED_IN || state == LOGGING_IN)
    {
        send_resp(fd, 530, NULL);
    }
    if (strcmp(cmd, "RNTO") == 0)
    {
        cmd_rnto(param, idx);
    }
    else if (state == WAITING_RNTO) 
    {
        send_resp(fd, 530, NULL);
    }
    if (strcmp(cmd, "RNFR") == 0)
    {
         cmd_rnfr(param, idx);
    }

    else if (strcmp(cmd, "TYPE") == 0)
    {
         cmd_type(param, idx);
    }
    else if (strcmp(cmd, "SYST") == 0)
    {
         cmd_syst(param, idx);
    }
    else if (strcmp(cmd, "QUIT") == 0)
    {
         cmd_quit(param, idx);
    }
    
    else if (strcmp(cmd, "PORT") == 0)
    {
         cmd_port(param, idx);
    }
    else if (strcmp(cmd, "PASV") == 0)
    {
         cmd_pasv(param, idx);
    }
    else if (strcmp(cmd, "STOR") == 0)
    {
         cmd_stor(param, idx);
    }
    else if (strcmp(cmd, "RETR") == 0)
    {
         cmd_retr(param, idx);
    }
    else if (strcmp(cmd, "PWD") == 0)
    {
         cmd_pwd(param, idx);
    }
    else if (strcmp(cmd, "CWD") == 0)
    {
         cmd_cwd(param, idx);
    }
    else if (strcmp(cmd, "MKD") == 0)
    {
         cmd_mkd(param, idx);
    }
    else if (strcmp(cmd, "RMD") == 0)
    {
         cmd_rmd(param, idx);
    }
    else if (strcmp(cmd, "DELE") == 0)
    {
         cmd_dele(param, idx);
    }
    else if (strcmp(cmd, "LIST") == 0)
    {
         cmd_list(param, idx);
    }
    else if (strcmp(cmd, "REST") == 0)
    {
         cmd_rest(param, idx);
    }
}

void cmd_user(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
    }
    else if (state != NOT_LOGGED_IN)
    {
        send_resp(fd, 530, NULL);
    }
    else if (strcmp(param, "anonymous") == 0)
    {
        clients[idx].state = LOGGING_IN;
        send_resp(fd, 331, NULL);
    }
    else
    {
        send_resp(fd, 530, NULL);
    }
}

void cmd_pass(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;
    if (state != LOGGING_IN)
    {
        send_resp(fd, 503, NULL);
        return;
    }
    clients[idx].state = LOGGED_IN;
    send_resp(fd, 230, NULL);
}

void cmd_syst(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    send_resp(fd, 215, NULL);
}

void cmd_type(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
    }
    else if (strcmp(param, "I") == 0)
    {
        send_resp(fd, 200, "Type set to I.");
    }
    else if (strcmp(param, "A") == 0)
    {
        send_resp(fd, 200, "Type set to I.");
    }
    else
    {
        send_resp(fd, 504, NULL);
    }
}

void cmd_quit(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    char byteInfo[100];
    char fileInfo[100];
    sprintf(fileInfo, "You have transferred %d files.", clients[idx].transfers);
    sprintf(byteInfo, "You have transferred %d bytes.", clients[idx].bytes);
    send_statistics(fd, 221, fileInfo, byteInfo, "goodbye!");
    //send_resp(fd, 221, NULL);
    close_trans_fd(idx);
    close_conn_fd(idx);
}

void cmd_abor(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    if (clients[idx].transfer_fd != -1)
    {
        close_trans_fd(idx);
        send_resp(fd, 426, NULL);
    }
    send_resp(fd, 226, NULL);
}

void cmd_pasv(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int trans_fd = clients[idx].transfer_fd;
    if (param != NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    // already exists a transfer connection
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
    
    sscanf(local_ip, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
    p1 = port / 256;
    p2 = port % 256;
    char resp[50];
    sprintf(resp, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)", h1, h2, h3, h4, p1, p2);

    clients[idx].mode = LISTENING;

    trans_fd = start_listening(port);
    if (trans_fd == -1)
    {
        send_resp(fd, 500, NULL);
        return;
    }

    manage_trans_fds(trans_fd, idx);
    send_resp(fd, 227, resp);
}

void cmd_port(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int trans_fd = clients[idx].transfer_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    int h1, h2, h3, h4, p1, p2;
    int count;
    count = sscanf(param, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
    if (count != 6)
    {
        send_resp(fd, 501, NULL);
        return;
    }
    if (h1 < 0 || h2 < 0 || h3 < 0 || h4 < 0 || p1 < 0 || p2 < 0 ||
        h1 > 255 || h2 > 255 || h3 > 255 || h4 > 255 || p1 > 255 || p2 > 255)
    {
        send_resp(fd, 501, NULL);
        return;
    }

    // already exists a transfer connection
    if (trans_fd != -1)
    {
        close_trans_fd(idx);
    }

    char ipaddr[30];

    sprintf(ipaddr, "%d.%d.%d.%d", h1, h2, h3, h4);
    clients[idx].mode = READY_TO_CONNECT;

    // set the ip and port
    memset(&(clients[idx].addr), 0, sizeof(clients[idx].addr));
    clients[idx].addr.sin_family = AF_INET;
    clients[idx].addr.sin_port = htons((p1 << 8) + p2);

    if (inet_pton(AF_INET, ipaddr, &(clients[idx].addr.sin_addr)) <= 0)
    {
        //printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        send_resp(fd, 501, NULL);
        return;
    }

    if ((trans_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        //printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        send_resp(fd, 500, NULL);
        return;
    }
    manage_trans_fds(trans_fd, idx);

    send_resp(fd, 200, "PORT command successful.");
}

int prepare_transfer(char *param, int idx) {
    
    int fd = clients[idx].connect_fd;
    int mode = clients[idx].mode;
    if (mode == NO_CONNECTION) // 有待考虑
    {
        send_resp(fd, 425, NULL);
        return 0;
    }
    if (mode == READY_TO_CONNECT)
    {
        //clients[idx].mode = TRANSFER_READY;
        clients[idx].mode = PORT_MODE;

        int trans_fd = clients[idx].transfer_fd;
        struct sockaddr_in addr = clients[idx].addr;
        if (connect(trans_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            //printf("Error connect(): %s(%d)\n", strerror(errno), errno);
            close(trans_fd);
            send_resp(fd, 425, NULL);
            return 0;
        }
        send_resp(fd, 150, NULL);
        gen_absdir(clients[idx].prefix, param, clients[idx].filename);
    }
    if (mode == LISTENING)
    {
        clients[idx].mode = PASV_MODE;
        gen_absdir(clients[idx].prefix, param, clients[idx].filename);
        send_resp(fd, 150, NULL);
    }
    return 1;
}

void cmd_retr(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    gen_absdir(clients[idx].prefix, param, clients[idx].filename);
    FILE *f;
    if ((f = fopen(clients[idx].filename, "rb+")) == NULL)
    {
        send_resp(fd, 550, NULL);
        return;
    }
    else
    {
        fclose(f);    
    }
    int i = prepare_transfer(param, idx);
    if (i != 1) 
    {
        return;
    }
    clients[idx].transfers += 1;
    clients[idx].rw = READ;
    clients[idx].state = FILE_TRANSFERING;
}

void cmd_stor(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    gen_absdir(clients[idx].prefix, param, clients[idx].filename);
    FILE *f;
    if ((f = fopen(clients[idx].filename, "ab+")) == NULL)
    {
        send_resp(fd, 550, NULL);
        return;
    }
    else
    {
        fclose(f);
    }
    int i = prepare_transfer(param, idx);
    if (i != 1)
    {
        return;
    }
    clients[idx].transfers += 1;
    clients[idx].rw = WRITE;
    clients[idx].state = FILE_TRANSFERING;
}

void cmd_rest(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    int start_pos = -1;
    if(sscanf(param, "%d", &start_pos) == 1 && start_pos >= 0)
    {
        clients[idx].start_pos = start_pos;
        send_resp(fd, 350, "REST set");
    }
    else
    {
        send_resp(fd, 501, NULL);
    }
}

void cmd_mkd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }

    if (strstr(param, "..") != NULL)
    {
        // ban..
        send_resp(fd, 530, NULL);
        return;
    }
    char dest[256];
    char resp[260];
    gen_absdir(clients[idx].prefix, param, dest);
    sprintf(resp, "\"%s\"", dest);
    if (mkdir(dest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) 
    {
        send_resp(fd, 250, resp);
    }
    else 
    {
        send_resp(fd, 550, NULL);
    }
}

void cmd_cwd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }

    if (strstr(param, "..") != NULL)
    {
        // ban..
        send_resp(fd, 530, NULL);
        return;
    }

    char dest[256];
    gen_absdir(clients[idx].prefix, param, dest);
    if (folder_isvalid(dest))
    {
        strcpy(clients[idx].prefix, dest);
        send_resp(fd, 250, NULL);
    }
    else
    {
        send_resp(fd, 550, NULL);
    }
}

void cmd_pwd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }
    char resp[260];
    // add double quotes
    sprintf(resp, "\"%s\"", clients[idx].prefix);
    send_resp(fd, 257, resp);
}

void cmd_list(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param != NULL)
    {
        char dest[256];
        gen_absdir(clients[idx].prefix, param, dest);
        if (!file_isvalid(dest) && !folder_isvalid(dest))
        {
            send_resp(fd, 451, NULL);
            return;
        }
        else
        {
            int i = prepare_transfer(dest, idx);
            if (i != 1)
            {
                return;
            }
            clients[idx].rw = LIST;
        }
    }
    else
    {
        int i = prepare_transfer(clients[idx].prefix, idx);
        if (i != 1)
        {
            return;
        }
        clients[idx].rw = LIST;
    }
}

void cmd_rmd(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }

    if (strstr(param, "..") != NULL)
    {
        // ban..
        send_resp(fd, 530, NULL);
        return;
    }
    char dest[256];
    gen_absdir(clients[idx].prefix, param, dest);
    if (rm_emptydir(dest) == 0)
    {
        send_resp(fd, 250, NULL);
    }
    else
    {
        send_resp(fd, 550, NULL);
    }
}

void cmd_dele(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }

    if (strstr(param, "..") != NULL)
    {
        // ban..
        send_resp(fd, 530, NULL);
        return;
    }
    char dest[256];
    gen_absdir(clients[idx].prefix, param, dest);
    if (recursive_rmdir(dest) == 0)
    {
        send_resp(fd, 250, NULL);
    }
    else
    {
        send_resp(fd, 550, NULL);
    }
}

void cmd_rnfr(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }

    if (strstr(param, "..") != NULL)
    {
        // ban..
        send_resp(fd, 530, NULL);
        return;
    }
    char dest[256];
    gen_absdir(clients[idx].prefix, param, dest);
    if (file_isvalid(dest))
    {
        clients[idx].state = WAITING_RNTO;
        strcpy(clients[idx].rename_file, dest);
        send_resp(fd, 350, NULL);
    }
    else
    {
        send_resp(fd, 550, NULL);
    }
}

void cmd_rnto(char *param, int idx)
{
    int fd = clients[idx].connect_fd;
    int state = clients[idx].state;
    clients[idx].state = LOGGED_IN;
    if (state != WAITING_RNTO) 
    {
        send_resp(fd, 503, "Previous command is not RNFR.");
        return;
    }
    if (param == NULL)
    {
        send_resp(fd, 504, NULL);
        return;
    }

    if (strstr(param, "..") != NULL)
    {
        // ban..
        send_resp(fd, 530, NULL);
        return;
    }
    char dest[256];
    gen_absdir(clients[idx].prefix, param, dest);

    char shell_cmd[512];
    sprintf(shell_cmd, "mv \"%s\" \"%s\"", clients[idx].rename_file, dest);
    if (system(shell_cmd) == -1)
    {
        //printf("Error running shell: %s(%d)\n", strerror(errno), errno);
        send_resp(fd, 550, NULL);
    }
    else
    {
        send_resp(fd, 250, NULL);
    }
}