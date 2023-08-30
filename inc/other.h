#ifndef _OTHER_H_
#define _OTHER_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <strings.h>
#include <pwd.h>
#include <grp.h>
#include <math.h>
// copy函数
void copyFile(char *sourcePath, char *destinationPath);
void copyDir(char *srcDir, char *dstDir);

// 进度条
int process(char *path, int size);

// 读取文件大小
int mystat(char *path);

int highNumber(int number);
int checkSize(char *fileName);

void *routinue(void *arg);
#endif