#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
void mystat(char *path)
{
    int ret;
    // 定义结构体变量，存放你要获取的那个文件的属性信息
    struct stat mystat;
    // stat函数的大小
    ret = stat(path, &mystat);
    if (ret == -1)
    {
        printf("获取文件属性失败了!\n");
        return -1;
    }
    printf("文件的大小是: %lu\n", mystat.st_size);
}

int main(int argc, char *argv[])
{
    char path[20] = {0};
    // strcpy(path, argv[1]);
    printf("请输入文件名称\n");
    scanf("%s", path);
    // printf("%s", path);
    mystat(path);
}