#include "cmds.h"
#include "server.h"
#include "dir_utils.h"
#include "fd_manager.h"
#include "global.h"
#include "soc_utils.h"
#include "rw_utils.h"

extern int listen_port;

extern char root_folder[256];
extern char local_ip[20];

int main(int argc, char **argv) {
    
    srand((int)time(0));
    int i;
    int nready;
    
    // default values for port and root directory
    strcpy(root_folder, "/tmp");
    listen_port = 21;
    
    int opt;
    char check_c;
    const struct option arg_options[] = 
    {
        {"port", required_argument, NULL, 'p'},

        {"root", required_argument, NULL, 'r'},

        {NULL, 0, NULL, 0},
    };

    while ((opt = getopt_long_only(argc, (char *const *)argv, "p:r:", arg_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'r':
            if (access(optarg, 0) == -1)
            {
                //printf("Directory doesn't exist: %s.\n", optarg);
                return 0;
            }
            strcpy(root_folder, optarg);
            break;
        case 'p':
            if (sscanf(optarg, "%d%c", &listen_port, &check_c) != 1)
            {
                //printf("Port number invalid: %s.\n", optarg);
                return 0;
            }
            if (listen_port < 1 || listen_port > 65535)
            {
                return 0;
            }
            break;
        case '?':
            //printf("Wrong argument.\n");
            return 0;
        }
    }
    // printf("%d%s\n", listen_port, root_folder);
    if (get_local_ip(local_ip) == -1)
    {
        //printf("Error getting local ip!");
        return 0;
    }

    fd_set rset, wset; // read and write fd_set

    int listen_fd = start_listening(listen_port);
    if (listen_fd == -1)
    {
        return 0;
    }
    init_fd_manager(listen_fd);
    
    while (1) {
        // assign the set of all active fds (allset) to rset and wset
        rset = allset;
        wset = allset;

        // use select to pick out the fds prepared to read/write
        nready = select(max_fd + 1, &rset, &wset, NULL, NULL);

        if (nready == -1) 
        {
            if (errno == EINTR) 
            {
                continue; // do not exit if the error is interrupted system call
            }
            else 
            {
                //printf("Error select(): %s(%d)\n", strerror(errno), errno);
                break;
            }
        }

        if (FD_ISSET(listen_fd, &rset)) {

            int connect_fd = accept(listen_fd, NULL, NULL);
            if (connect_fd == -1)
            {
                //printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            }
            else if (!manage_conn_fds(connect_fd))
            {
                close_conn_fd(connect_fd); // if fd num reaches upper limit
            }
            send_resp(connect_fd, 220, NULL); // welcome message
            if (--nready <= 0) 
            {
                continue; // no more ready fds
            }
        }
        for (i = 0; i <= max_i; i++) 
        {
            int tmp_fd = clients[i].connect_fd;
            if (tmp_fd == -1)
            {
                continue;
            }
            if (FD_ISSET(tmp_fd, &rset)) {
                if(serve_client(tmp_fd, i) == 0)
                {
                    close_trans_fd(i);
                    close_conn_fd(i);
                }
                if (--nready <= 0)
                {
                    continue; // no more ready fds
                }
            }

            tmp_fd = clients[i].transfer_fd;
            if (tmp_fd == -1)
            {
                continue; // no tranfer fd open
            }
            if (FD_ISSET(tmp_fd, &rset)){
                int mode = clients[i].mode;

                // if in pasv mode, a fd ready to read means a connection
                if (mode == PASV_MODE)
                {
                    int transfd = accept(tmp_fd, NULL, NULL);
                    if (transfd == -1)
                    {
                        //printf("Error accept(): %s(%d)\n", strerror(errno), errno);
                        send_resp(clients[i].connect_fd, 425, NULL);
                        close_trans_fd(i);
                        continue;
                    }
                    // replace the listen fd to a transfer fd
                    int start_pos = clients[i].start_pos;
                    close_trans_fd(i);
                    manage_trans_fds(transfd, i);
                    clients[i].start_pos = start_pos;
                    clients[i].mode = TRANSFER_READY;
                    clients[i].state = FILE_TRANSFERING;                    
                }
                if (mode == PORT_MODE)
                {
                    clients[i].mode = TRANSFER_READY;
                }
                if (mode == TRANSFER_READY)
                {
                    if (clients[i].rw == WRITE)
                    {
                        upload_file(i);
                    }
                    if (clients[i].rw == READ)
                    {
                        char sentence[8];
                        int len = recv(tmp_fd, sentence, 8, MSG_DONTWAIT);
                        // empty string means connection loss
                        if (len <= 0)
                        {
                            close_trans_fd(i);
                            send_resp(clients[i].connect_fd, 426, NULL);
                            continue;
                        }
                    }
                }
                if (--nready <= 0)
                {
                    continue; // no more ready fds
                }
            }
            else if (FD_ISSET(tmp_fd, &wset)) {
                
                int mode = clients[i].mode;
                
                if (mode == TRANSFER_READY)
                {
                    if(clients[i].rw == READ) 
                    {
                        download_file(i);       
                    }
                    else if (clients[i].rw == LIST) {
                        reply_list(i, clients[i].filename);
                    }
                }
                if (--nready <= 0)
                {
                    continue; // no more ready fds
                }
            }
        }
    }

    close(listen_fd); // don't forget to close it!
}

int serve_client(int fd, int idx)
{

    char sentence[1024];
    char param_check;
    int len;
    char cmd[10];
    char param[100];
    len = recv(fd, sentence, 1024, 0);

    // empty string means connection loss
    if (len <= 0)
    {
        return 0;
    }

    // preprocess
    sentence[len] = '\0';
    len = strip_crlf(sentence, len);

    // match contents
    int word_count = sscanf(sentence, "%s %c", cmd, &param_check);;
    if (word_count <= 0)
    {
        //printf("Empty command.\n");
        send_resp(fd, 500, NULL);
    }

    else if (word_count == 1)
    {
        // no params
        cmd_router(cmd, NULL, idx);
    }
    else
    {
        // has a param
        strcpy(param, sentence + strlen(cmd) + 1);
        cmd_router(cmd, param, idx);
    }
    return 1;
}

void download_file(int idx)
{
    if(clients[idx].state != FILE_TRANSFERING)
    {
        return;
    }
    int code = 226;
    FILE *f;
    int nbytes;
    char buf[BUFSIZE];
    memset(&buf, 0, BUFSIZE);
    char *filename = clients[idx].filename;
    if ((f = fopen(filename, "rb")) == NULL)
    {
        //printf("Error fopen(): %s(%d)\n", strerror(errno), errno);
        code = 451;
    }
    else
    {
        fseek(f, clients[idx].start_pos, SEEK_SET);
        if ((nbytes = fread(buf, 1, BUFSIZE, f)) > 0) 
        {
            if (!safe_send(clients[idx].transfer_fd, buf, nbytes))
            {
                code = 426;
            }
            else
            {
                clients[idx].bytes += nbytes;
                clients[idx].start_pos += nbytes;
                fclose(f);
                return;
            }
        }
        fclose(f);
    }
    close_trans_fd(idx);
    send_resp(clients[idx].connect_fd, code, NULL);
}

void upload_file(int idx)
{
    if (clients[idx].state != FILE_TRANSFERING)
    {
        return;
    }
    int code = 226;
    FILE *f;
    int nbytes;
    char buffer[BUFSIZE];
    memset(&buffer, 0, BUFSIZE);
    char *filename = clients[idx].filename;
    if (clients[idx].start_pos == 0)
    {
        f = fopen(filename, "wb");
    }
    else
    {
        f = fopen(filename, "rb+");
    }
    if (f == NULL)
    {
        //printf("Error fopen(): %s(%d)\n", strerror(errno), errno);
        code = 550;
    }
    else
    {
        fseek(f, clients[idx].start_pos, SEEK_SET);
        if ((nbytes = safe_recv(clients[idx].transfer_fd, buffer, BUFSIZE)) > 0)
        {
            if (fwrite(buffer, 1, nbytes, f) < nbytes)
            {
                //printf("Error fwrite(): %s(%d)\n", strerror(errno), errno);
                fflush(f);
                code = 451;
            }
            else
            {
                clients[idx].start_pos += nbytes;
                clients[idx].bytes += nbytes;
                fclose(f);
                return;
            }        
        }
        else if (nbytes < 0) 
        {
            code = 426;
        }
        fclose(f);
    }
    close_trans_fd(idx);
    send_resp(clients[idx].connect_fd, code, NULL);
}

void reply_list(int idx, char *dest)
{
    int fd = clients[idx].connect_fd;
    int trans_fd = clients[idx].transfer_fd;

    char list_res[4096];
    FILE *fp = NULL;
    char cmd[256];

    sprintf(cmd, "ls -l \"%s\"", dest);
    fp = popen(cmd, "r");
    if (fp != NULL)
    {
        // read the first line
        while (fread(list_res, 1, 1, fp))
        {
            if (list_res[0] == '\n')
            {
                break;
            }
        }
        while (1)
        {
            int read_size = fread(list_res, 1, 4096, fp);
            if (read_size == 0)
            {
                break;
            }
            else
            {
                list_res[read_size] = '\0';
            }

            safe_send(trans_fd, list_res, strlen(list_res));
        }
        pclose(fp);
    }
    close_trans_fd(idx);
    send_resp(fd, 226, NULL);
}