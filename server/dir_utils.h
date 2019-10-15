#ifndef DIR_UTIL_H
#define DIR_UTIL_H

// directory/file utils

void gen_absdir(char *prefix, char *dir, char *dest);

int file_isvalid(char *pth);

int folder_isvalid(char *pth);

int recursive_rmdir(char *pth);

#endif