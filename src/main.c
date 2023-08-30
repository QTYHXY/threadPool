#include <stdio.h>
#include "../inc/thread_pool.h"
#include "../inc/other.h"
char copyFile1[10];
char copyFile2[10];
char copyDir1[10];
char copyDir2[10];
// 创建线程池
ThreadPool *pool;

void copyFileTask(void *arg)
{
    // process();
    copyFile(copyFile1, copyFile2);
    sleep(5);
}
void copyDirTask(void *arg)
{
    // process();
    printf("正在检查%s路径下文件\n", copyDir1);
    printf("\n");
    sleep(1);
    printf("检测完成,开始拷贝%s目录\n", copyDir1);
    printf("\n");
    copyDir(copyDir1, copyDir2);
    sleep(5);
}

int main()
{
    ThreadPool *pool = threadPoolCreate(2, 10, 10);

    int number = 0;
    // int *num = (int *)malloc(sizeof(int));
    printf("1.拷贝文件\n2.拷贝目录\n3.拷贝文件和目录同时进行\n");
    scanf("%d", &number);
    switch (number)
    {
    case 1:
        printf("输入要拷贝的文件名\n");
        scanf("%s", copyFile1);

        printf("输入目的文件文件名\n");
        scanf("%s", copyFile2);
        // *num = 1;
        printf("\n");
        threadPoolAdd(pool, copyFileTask, NULL);
        break;
    case 2:
        printf("输入要拷贝的目录名\n");
        scanf("%s", copyDir1);

        printf("输入目的路径的目录名\n");
        scanf("%s", copyDir2);
        // *num = 2;
        printf("\n");
        threadPoolAdd(pool, copyDirTask, NULL);
        break;
    case 3:
        printf("输入要拷贝的文件名\n");
        scanf("%s", copyFile1);

        printf("输入目的文件文件名\n");
        scanf("%s", copyFile2);

        printf("输入要拷贝的目录名\n");
        scanf("%s", copyDir1);

        printf("输入目的路径的目录名\n");
        scanf("%s", copyDir2);

        printf("\n");
        threadPoolAdd(pool, copyDirTask, NULL);
        threadPoolAdd(pool, copyFileTask, NULL);

        break;
    default:
        printf("输入错误，退出程序\n");
        exit(0);
        break;
    }

    sleep(30);
    threadPoolDestroy(pool);
    return 0;
}