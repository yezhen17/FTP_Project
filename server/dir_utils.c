#include "dir_utils.h"
#include "cmds.h"
#include "global.h"


extern struct client_info clients[MAX_CLIENTS];
extern int max_i, max_fd;
extern fd_set allset;





void gen_absdir(char *prefix, char *dir, char *dest)
{
    int len = strlen(prefix);

    // 绝对路径
    if (dir[0] == '/')
    {        
        sprintf(dest, "%s", dir);
    }
    // 相对路径
    else
    {
        if (prefix[len - 1] == '/')
            sprintf(dest, "%s%s", prefix, dir);
        else
            sprintf(dest, "%s/%s", prefix, dir);
    }
}

// 成功返回0，失败返回-1
int recursive_rmdir(char *pth) {
    char child_dir[200];
    DIR *dir_p;
    struct dirent *dp;
    struct stat dir_stat;

    //目录不存在，返回-1
    if (access(pth, 0) != 0)
        return -1;

    //获取目录属性失败
    if (stat(pth, &dir_stat) < 0)
        return -1;

    //是一个普通文件，直接删除
    if (S_ISREG(dir_stat.st_mode))
    {
        if (remove(pth) != 0)
            return -1;
        return 0;
    }

    //是一个目录，需要进行递归删除
    else if (S_ISDIR(dir_stat.st_mode))
    {
        dir_p = opendir(pth);
        while ((dp = readdir(dir_p)) != NULL)
        {
            // 忽略 . 和 ..
            if (strcmp(".", dp->d_name) == 0 || strcmp("..", dp->d_name) == 0)
                continue;

            sprintf(child_dir, "%s/%s", pth, dp->d_name);
            // 递归调用
            if (recursive_rmdir(child_dir) != 0)
                return -1;
        }
        closedir(dir_p);

        // 只剩余空目录，删除之
        if (rmdir(pth) != 0)
            return -1;
        return 0;
    }
    else
        return -1;
}

int file_isvalid(char *pth) {
    struct stat file_stat;
    int valid = 1;
    if (access(pth, 0) != 0)
        valid = 0;
    else if (stat(pth, &file_stat) < 0)
        valid = 0;
    else if (!S_ISREG(file_stat.st_mode))
        valid = 0;
    return valid;
}

int folder_isvalid(char *pth) {
    struct stat file_stat;
    int valid = 1;
    if (access(pth, 0) != 0)
        valid = 0;
    else if (stat(pth, &file_stat) < 0)
        valid = 0;
    else if (!S_ISDIR(file_stat.st_mode))
        valid = 0;
    return valid;
}

int reply_list(int idx, char *dest) {
    int fd = clients[idx].connect_fd;
    int trans_fd = clients[idx].transfer_fd;
    //开始处理
    char list_res[4096];
    FILE *fp = NULL;
    char cmd[256];

    sprintf(cmd, "ls -l \"%s\"", dest);
    fp = popen(cmd, "r");
    if (fp)
    {
        // 首先读出一行
        while (fread(list_res, 1, 1, fp))
        {
            if (list_res[0] == '\n')
                break;
        }
        while (1)
        {
            int read_size = fread(list_res, 1, 4096, fp);
            if (read_size == 0)
                break;
            else
                list_res[read_size] = '\0';
            safe_send(trans_fd, list_res, strlen(list_res));
        }
        pclose(fp);
    }
    //传输完成
    close_trans_fd(idx);
    send_resp(fd, 226, NULL);
}