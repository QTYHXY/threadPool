#include "../inc/other.h"
#define NUM 51

int process(char *path, int size)
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
    // printf("文件的大小是: %lu\n", mystat.st_size);

    int sleepTime;

    sleepTime = size / 200;

    char buffer[NUM + 1] = {0}; // 存储进度条字符
    char arr[5] = {"-/|\\"};    // 存储基本的变化字幕
    for (int i = 0; i < NUM; ++i)
    {
        buffer[i] = '>';
        printf("[%-51s][%d%%]\r", buffer, i * 2);
        if (i * 2 == 100)
        {
            printf("[%s]\r", "完成");
        }
        else
        {
            printf("[%c]\r", arr[i % 4]);
            fflush(stdout);
        }
        usleep(sleepTime);
    }
    printf("\n");
    return 0;
}

// int main(int argc, char *agrv[])
// {
//     char path[20] = "qwer.pdf";
//     process(path);
// }