#include "cmds.h"
#include "server.h"
#include "dir_utils.h"
#include "fd_manager.h"
#include "global.h"
#include "soc_utils.h"
#include "rw_utils.h"

extern int listen_port;
char check_s;
extern char root_folder[256];
extern char local_ip[20];

int main(int argc, char **argv) {
    
    srand((int)time(0));
    int i;
    int nready;
    
    // default values for port and root directory
    strcpy(root_folder, "/home/cyz/tmp");
    listen_port = 9999;
    
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
            if (access(optarg, 0) == -1)
            {
                printf("Directory doesn't exist: %s.\n", optarg);
                return 0;
            }
            strcpy(root_folder, optarg);
            break;
        case 'p':
            if (sscanf(optarg, "%hd%c", &listen_port, &check_s) != 1)
            {
                printf("Port number invalid: %s.\n", optarg);
                return 0;
            }
            break;
        case '?':
            printf("Wrong argument.\n");
            return 0;
        }
    }
    printf("%d\n", listen_port);
    printf("%s\n", root_folder);

    if (get_local_ip(local_ip) == -1)
    {
        printf("Error getting local ip!");
        return 0;
    }

    struct sockaddr_in addr;
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
                printf("Error select(): %s(%d)\n", strerror(errno), errno);
                break;
            }
        }

        if (FD_ISSET(listen_fd, &rset)) {

            int connect_fd = accept(listen_fd, NULL, NULL);
            if (connect_fd == -1)
            {
                printf("Error accept(): %s(%d)\n", strerror(errno), errno);
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
                        printf("Error accept(): %s(%d)\n", strerror(errno), errno);
                        continue;
                    }
                    // replace the listen fd to a transfer fd
                    int start_pos = clients[i].start_pos;
                    close_trans_fd(i);
                    manage_trans_fds(transfd, i);
                    clients[i].start_pos = start_pos;
                    clients[i].mode = TRANSFER_READY;
                    clients[i].state = FILE_TRANSFERING;
                    printf("ready to transfer\n");
                    
                }
                if (mode == TRANSFER_READY)
                {
                    if (clients[i].rw == WRITE)
                    {
                        upload_file(i);
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

void download_file(int idx)
{
    if(clients[idx].state != FILE_TRANSFERING)
    {
        return;
    }
    int code = 226;
    int stop = 1;
    FILE *f;
    int nbytes;
    char buf[BUFSIZE];
    memset(&buf, 0, BUFSIZE);
    char *filename = clients[idx].filename;
    //gen_absdir(clients[idx].prefix, clients[idx].filename, filename);
    printf("get filename : [ %s ]\n", filename);
    if ((f = fopen(filename, "rb")) == NULL)
    {
        printf("Error fopen(): %s(%d)\n", strerror(errno), errno);
        code = 451;
    }
    else
    {
        printf("%d\n",clients[idx].start_pos);
        fseek(f, clients[idx].start_pos, SEEK_SET);
        if ((nbytes = fread(buf, 1, BUFSIZE, f)) > 0) //���ļ����ݶ���buffer��
        {
            //printf("%s", buf);
            if (!safe_send(clients[idx].transfer_fd, buf, nbytes))
            {
                code = 451;
                //break;
            }
            clients[idx].start_pos += nbytes;
            
            fclose(f);
            return;
            //break;
        }
        fclose(f);
    }
    close_trans_fd(idx);
    send_resp(clients[idx].connect_fd, code, NULL);
}

int c = 0;
void upload_file(int idx)
{
    if (clients[idx].state != FILE_TRANSFERING)
    {
        return;
    }
    int code = 226;
    int start_pos = clients[idx].start_pos;
    FILE *f;
    int nbytes;
    char buffer[BUFSIZE];
    memset(&buffer, 0, BUFSIZE);
    char *filename = clients[idx].filename;
    //gen_absdir(clients[idx].prefix, clients[idx].filename, filename);
    //printf("%s\n", filename);
    if (start_pos == -1)
    {
        f = fopen(filename, "wb");
    }
    else
    {
        f = fopen(filename, "ab+");
    }
    if (f == NULL)
    {
        printf("Error fopen(): %s(%d)\n", strerror(errno), errno);
        code = 451;
    }
    else
    {
        c++;
        //fseek(f, clients[idx].start_pos, SEEK_SET);
        //printf("%d", clients[i].start_pos);
        if ((nbytes = safe_recv(clients[idx].transfer_fd, buffer, BUFSIZE)) > 0) //���ļ����ݶ���buffer��
        {
            //printf("a line\n");
            //printf("%s", buffer);
            if (fwrite(buffer, 1, nbytes, f) < nbytes) //��buffer���ͻ�client
            {
                printf("Error fwrite(): %s(%d)\n", strerror(errno), errno);
                fflush(f);
                code = 530;
                //break;
            }
            //clients[idx].start_pos += nbytes;
            //printf("%d", nbytes);
            fclose(f);
            return;
        }
        else if (nbytes < 0) 
        {
            code = 500;
        }
        fclose(f);
    }
    printf("%d\n", c);
    close_trans_fd(idx);
    send_resp(clients[idx].connect_fd, code, NULL);
}