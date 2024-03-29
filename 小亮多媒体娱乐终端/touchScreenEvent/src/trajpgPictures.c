#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "datatype.h"
#include "list.h"


#define IS_DIR 4
#define IS_REG 8

dataType getdata(const char *data1)
{
    dataType Data;
    strncpy(Data.name, data1, 100);
    strncpy(Data.type, ".jpg", 20);
    return Data;
}


pNode trajpgPic(const char *dirPath)
{
    //初始化头节点
    pNode head_pic = getNewNode(NULL);


    struct stat buf;
    bzero(&buf, sizeof(buf));
    
    //判断文件属性是否读取成功
    if (stat(dirPath, &buf) != 0)
    {
        perror("stat error");
        return NULL;
    }
    

    //判断是否是目录
    if (!S_ISDIR(buf.st_mode))//成功返回0
    {
        printf("%s is not a directory\n", dirPath);
        return NULL;
    }
    
    //打开目录文件dir
    DIR *dir = opendir(dirPath);

    if (dir == NULL)
    {
        perror("open dir error");
        return NULL;
    }

    struct dirent *tmp;
    while (1)
    {
        // 读取目录项 （每当读取一个目录项他的读写位置会自动往后偏移）
        tmp = readdir(dir);
        if (tmp == NULL)
        {
            break;
        }

        char tmpPath[1024] = {0};

        if (tmp->d_type == IS_DIR&&tmp->d_name[0] != '.')//'.'和'..'是每个目录都有的，如果等于就进入了无限循环
        {
            /*如果是目录，则需要递归到该目录内部去 */
            snprintf(tmpPath,1024,"%s/%s",dirPath,tmp->d_name);
            trajpgPic(tmpPath);
        }
         /*如果是普通文件，则判断是否是.c文件 */
        else if (tmp->d_type == IS_REG)
        {
            char *ptr = strrchr(tmp->d_name, '.');
            //必须判断是否为空，要不然strcmp函数接收的字符指针是NULL会发生段错误
            if (ptr == NULL)
            {
                continue;
            }
            char tmpPath[1024] = {0};
            if (strcmp(ptr, ".jpg") == 0)
            {
                // printf("%s/%s\n",dirPath ,tmp->d_name);
                snprintf(tmpPath,1024,"%s/%s",dirPath,tmp->d_name);
                dataType newData = getdata(tmpPath);
                pNode newNode = getNewNode(&newData);
                addlist(head_pic->prev,newNode,head_pic);
            }
            
        }
        
    }

    //关闭目录
    closedir(dir);
    return head_pic;
    
}