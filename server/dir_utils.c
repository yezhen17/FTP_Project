#include "dir_utils.h"
#include "cmds.h"
#include "global.h"
#include "rw_utils.h"

extern struct client_info clients[MAX_CLIENTS];
extern int max_i, max_fd;
extern fd_set allset;

void gen_absdir(char *prefix, char *dir, char *dest)
{
    int len = strlen(prefix);
    // dir is absolute
    if (dir[0] == '/')
    {        
        sprintf(dest, "%s", dir);
    }
    // dir is relative
    else if (dir[0] == '.')
    {
        if (prefix[len - 1] == '/')
        {
            sprintf(dest + 2, "%s%s", prefix, dir);
        }
        else
        {
            sprintf(dest + 2, "%s/%s", prefix, dir);
        }
    }
    else
    {
        if (prefix[len - 1] == '/')
        {
            sprintf(dest, "%s%s", prefix, dir);
        }
        else
        {
            sprintf(dest, "%s/%s", prefix, dir);
        }
    }
}

// if success return 0, else -1
int rm_emptydir(char *pth)
{
    struct stat dir_stat;

    // dir not exist
    if (access(pth, 0) == -1)
    {
        return -1;
    }
    // fail to get the dir stat
    if (stat(pth, &dir_stat) < 0)
    {
        return -1;
    }
    // target is a regular file
    if (S_ISREG(dir_stat.st_mode))
    {
        return -1;
    }
    // target is a folder
    else if (S_ISDIR(dir_stat.st_mode))
    {
        if (rmdir(pth) != 0)
            return -1;
        return 0;
    }
    // else, just ignore
    else
    {
        return -1;
    }
}

// if success return 0, else -1
int recursive_rmdir(char *pth) {
    char child_dir[512]; // prevent warning
    DIR *dir_p;
    struct dirent *dp;
    struct stat dir_stat;

    // dir not exist
    if (access(pth, 0) == -1)
    {
        return -1;
    }
    // fail to get the dir stat
    if (stat(pth, &dir_stat) < 0)
    {
        return -1;
    }
    // target is a regular file
    if (S_ISREG(dir_stat.st_mode))
    {
        if (remove(pth) != 0)
            return -1;
        return 0;
    }
    // target is a folder
    else if (S_ISDIR(dir_stat.st_mode))
    {
        dir_p = opendir(pth);
        while ((dp = readdir(dir_p)) != NULL)
        {
            // jump . and ..
            if (strcmp(".", dp->d_name) == 0 || strcmp("..", dp->d_name) == 0)
            {
                continue;
            }
            sprintf(child_dir, "%s/%s", pth, dp->d_name);
            // recursive call
            if (recursive_rmdir(child_dir) != 0)
                return -1;
        }
        closedir(dir_p);

        // last remove the empty folder
        if (rmdir(pth) != 0)
            return -1;
        return 0;
    }
     // else, just ignore
    else
    {
        return -1;
    }
}

// return 1 if file is valid,, else 0
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

// return 1 if folder is valid, else 0
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
