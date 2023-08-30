#include "../inc/other.h"
#include "../inc/thread_pool.h"

extern char copyFile1[10];
extern char copyFile2[10];
extern char copyDir1[10];
extern char copyDir2[10];

extern ThreadPool *pool;
int fileSize;
int highPlace;

typedef struct fileArg
{
    /* data */
    FILE *fileNameScoure;
    FILE *fileDestination;
    int size;
    int count;
} fileArg;

void *routinue(void *arg)
{
    fileArg fileArgNum = *(fileArg *)arg;
    // printf("我是第%lu条线程\n", pthread_self());
    // printf("fileNameScoure=%d\n", fileArgNum.fileNameScoure);
    // printf("fileDestination=%d\n", fileArgNum.fileDestination);
    // printf("size=%d\n", fileArgNum.size);
    // printf("count=%d\n", fileArgNum.count);
    char buffer[2048000] = {0};
    // int count = fileArgNum.count;
    // printf("我负责的是%d到%d部分\n", fileArgNum.count * 2048000, (fileArgNum.count + 1) * 2048000 - 1);
    fread(buffer, 2047999, 1, fileArgNum.fileNameScoure);
    fseek(fileArgNum.fileNameScoure, fileArgNum.count * 2048000, (fileArgNum.count * 1) * 2048000 - 1);
    fwrite(buffer, 2047999, 1, fileArgNum.fileDestination);
    fseek(fileArgNum.fileDestination, fileArgNum.count * 2048000, (fileArgNum.count * 1) * 2048000 - 1);
    // printf("\n");
    // return;
}

void copyFile(char *sourcePath, char *destinationPath)
{
    printf("正在准备拷贝文件%s\n", sourcePath);
    FILE *source = fopen(sourcePath, "rb");
    FILE *destination = fopen(destinationPath, "wb");

    if (source == NULL || destination == NULL)
    {
        fprintf(stderr, "无法复制文件 %s\n", sourcePath);
        return;
    }

    int ch;
    int count;
    while ((ch = fgetc(source)) != EOF)
    {
        fputc(ch, destination);
        count++;
    }
    // printf("count=%d\n", count);
    process(sourcePath, count);
    printf("文件%s拷贝完成\n", sourcePath);
    printf("\n");
    fclose(source);
    fclose(destination);
}

int highNumber(int number)
{
    highPlace = 0;
    if (number % 10 == 0)
        number = number / 10;
    while (!(number >= 0 && number <= 9))
    {
        number = number / 10;
        highPlace++;
    }
    // printf("%d\n", number);
    return highPlace + 1;
}

int checkSize(char *fileName)
{

    int ret;
    // 定义结构体变量，存放你要获取的那个文件的属性信息
    struct stat mystat;
    // stat函数的大小
    ret = stat(fileName, &mystat);
    if (ret == -1)
    {
        printf("获取文件属性失败了!\n");
        return -1;
    }
    fileSize = mystat.st_size;
    // printf("文件的大小是: %lu\n", mystat.st_size);
    return fileSize;
}

// void copyFile(char *sourcePath, char *destinationPath)
// {
//     fileArg fileArg;
//     printf("正在准备拷贝文件%s\n", sourcePath);
//     FILE *source = fopen(sourcePath, "rb");
//     FILE *destination = fopen(destinationPath, "wb");

//     if (source == NULL || destination == NULL)
//     {
//         fprintf(stderr, "无法复制文件 %s\n", sourcePath);
//         return;
//     }
//     // 查看文件大小
//     int fileSize = checkSize(sourcePath);
//     // printf("%s的文件大小是%d\n", sourcePath, fileSize);
//     int highnum = highNumber(fileSize);
//     // printf("%s的文件大小的最高位是%d\n", sourcePath, highnum);
//     // printf("fileName=%d\n", source);

//     if (highnum >= 8)
//     {
//         int i;
//         pthread_t tid;

//         // printf("分为%d段\n", fileSize / 2048000 + 1);
//         fileArg.fileNameScoure = source;
//         fileArg.fileDestination = destination;
//         fileArg.size = fileSize;
//         while (i <= fileSize / 2048000)
//         {
//             fileArg.count = i;
//             pthread_create(&tid, NULL, routinue, (void *)&fileArg);
//             // char buffer[2048000] = {0};
//             // fread(buffer, 2048000, (fileSize / 2048000 + 1), source);
//             // fwrite(buffer, 2048000, fileSize / 2048000 + 1, destination);
//             // printf("第%d段\n", i);
//             sleep(0.5);
//             i++;
//         }
//         // fread(buffer, 1024, , fp);
//     }
//     else
//     {
//         int ch;
//         while ((ch = fgetc(source)) != EOF)
//         {
//             fputc(ch, destination);
//         }
//     }

//     printf("文件%s拷贝完成\n", sourcePath);
//     printf("\n");
//     // sleep(5);
//     fclose(source);
//     fclose(destination);
// }

void copyDir(char *srcDir, char *dstDir)
{
    struct dirent *entry;
    DIR *dp = opendir(srcDir);

    if (dp == NULL)
    {
        fprintf(stderr, "无法打开源目录\n");
        return;
    }

    mkdir(dstDir, 0777);

    char srcPath[PATH_MAX];
    char dstPath[PATH_MAX];

    while ((entry = readdir(dp)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(srcPath, sizeof(srcPath), "%s/%s", srcDir, entry->d_name);
        snprintf(dstPath, sizeof(dstPath), "%s/%s", dstDir, entry->d_name);

        struct stat st;
        stat(srcPath, &st);

        if (S_ISDIR(st.st_mode))
        {
            copyDir(srcPath, dstPath);
            // threadPoolAdd(pool, copyFileTask, NULL);
        }
        else if (S_ISREG(st.st_mode))
        {
            // threadPoolAdd(pool, copyFileTask, NULL);
            copyFile(srcPath, dstPath);
        }
        else
        {
            fprintf(stderr, "无法复制 %s, 已略过\n", srcPath);
        }
    }
    // process(srcPath);
    printf("目录%s拷贝完成\n", srcDir);
    printf("\n");
    closedir(dp);
}

// int main()
// {
//     const char *sourceDir = "789";
//     const char *destinationDir = "999";

//     copyDir(sourceDir, destinationDir);

//     return 0;
// }