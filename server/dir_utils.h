/*
directory related utils
*/

#ifndef DIR_UTILS_H
#define DIR_UTILS_H

// generate absolute directory in dest
void gen_absdir(char *prefix, char *dir, char *dest);

// check whether the path belongs to a regular file
int file_isvalid(char *pth);

// check whether the path belongs to a folder
int folder_isvalid(char *pth);

// remove a directory recursively
int recursive_rmdir(char *pth);

// remove an empty folder
int rm_emptydir(char *pth);

#endif