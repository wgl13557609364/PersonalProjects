#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "list.h"
#include "trajpgPic.h"
#include "Playjpg.h"
#include "font.h"
#include <math.h>
#include <strings.h>
#define MAX_BUF_SIZE 400

void getTouchxy(int *x_in, int *y_in, int *x_out, int *y_out)
{
    // 打开触摸屏文件
    int fd_ts = open("/dev/input/event0", O_RDWR);
    if (fd_ts == -1)
    {
        perror("open event 0 error");
        return;
    }

    struct input_event tsEvent;
    int x_tmp = 0;
    int y_tmp = 0;

    while (1)
    {
        int ret_val = read(fd_ts, &tsEvent, sizeof(tsEvent));
        // printf("成功读取：%d字节数据..\n" , ret_val);

        if (tsEvent.type == EV_ABS) // 判断是否为触摸屏事件
        {
            if (tsEvent.code == ABS_X) // 判断是否为 x轴事件
            {
                x_tmp = tsEvent.value * 800 / 1024;
            }
            else if (tsEvent.code == ABS_Y) // 判断是否为 x轴事件
            {
                y_tmp = tsEvent.value * 480 / 600;
            }
        }

        if (tsEvent.type == EV_KEY &&  // 按键事件
            tsEvent.code == BTN_TOUCH) // 被触摸事件
        {
            if (tsEvent.value == 1) // 按下
            {
                *x_in = x_tmp;
                *y_in = y_tmp;
            }
            else if (tsEvent.value == 0) // 松手
            {
                *x_out = x_tmp;
                *y_out = y_tmp;
                break; // 获取到坐标后跳出循环
            }
        }
    }

    // 关闭释放
    close(fd_ts);
}

// 滑动
void getSwipexy(int *x_start, int *y_start, int *x_end, int *y_end)
{
    // 打开触摸屏文件
    int fd_ts = open("/dev/input/event0", O_RDWR);
    if (fd_ts == -1)
    {
        perror("open event 0 error");
        return;
    }

    struct input_event tsEvent;
    int x_tmp = 0;
    int y_tmp = 0;
    bool swipe_started = false;

    while (1)
    {
        int ret_val = read(fd_ts, &tsEvent, sizeof(tsEvent));

        if (tsEvent.type == EV_ABS) // 判断是否为触摸屏事件
        {
            if (tsEvent.code == ABS_X) // 判断是否为 x轴事件
            {
                x_tmp = tsEvent.value * 800 / 1024;
            }
            else if (tsEvent.code == ABS_Y) // 判断是否为 y轴事件
            {
                y_tmp = tsEvent.value * 480 / 600;
            }
        }

        if (tsEvent.type == EV_KEY && tsEvent.code == BTN_TOUCH)
        {
            if (tsEvent.value == 1) // 触摸开始
            {
                *x_start = x_tmp;
                *y_start = y_tmp;
                swipe_started = true;
            }
            else if (tsEvent.value == 0 && swipe_started) // 触摸结束
            {
                *x_end = x_tmp;
                *y_end = y_tmp;
                break; // 获取到坐标后跳出循环
            }
        }
    }

    // 关闭释放
    close(fd_ts);
}

// 根据坐标值进行相应处理
// 相册
void picture()
{

    // 把相册存入节点
    pNode head_photo = trajpgPic("./img/photo");
    pNode tmp_photo = head_photo;

    // 把菜单图片存入节点
    pNode head_muen = trajpgPic("./img/muen");

    // 把退出图标存入节点
    pNode head_exit = trajpgPic("./img/exit1");

    // 播放相册
    printf("相册\n");
    PlayJpg(tmp_photo->next, 0, 0);
    tmp_photo = tmp_photo->next;
    if (tmp_photo->next == head_photo)
    {
        tmp_photo = tmp_photo->next;
    }
    PlayJpg(head_exit->next, 0, 730);

    int x_start, y_start, x_end, y_end;
    getSwipexy(&x_start, &y_start, &x_end, &y_end);

    // 退出
    if (x_start >= 730 && x_start <= 799 && y_start >= 0 && y_start <= 69)
    {
        // 显示菜单
        PlayJpg(head_muen->next, 0, 0);
        // 释放资源
        destroyList(head_photo);
        destroyList(head_exit);
        destroyList(head_muen);
        return;
    }
    else
    {
        while (1)
        {

            if (x_start - x_end >= 10)
            {
                if (tmp_photo->next == head_photo)
                {
                    tmp_photo = tmp_photo->next;
                }
                PlayJpg(tmp_photo->next, 0, 0);
                PlayJpg(head_exit->next, 0, 730);
                tmp_photo = tmp_photo->next;
                if (tmp_photo->next == head_photo)
                {
                    tmp_photo = tmp_photo->next;
                }
            }

            if (x_start - x_end <= -10)
            {
                if (tmp_photo->prev == head_photo)
                {
                    tmp_photo = tmp_photo->prev;
                }
                PlayJpg(tmp_photo->prev, 0, 0);
                PlayJpg(head_exit->next, 0, 730);
                tmp_photo = tmp_photo->prev;
                if (tmp_photo->prev == head_photo)
                {
                    tmp_photo = tmp_photo->prev;
                }
            }

            // 退出循环条件
            getSwipexy(&x_start, &y_start, &x_end, &y_end);
            // 退出
            if (x_start >= 730 && x_start <= 799 && y_start >= 0 && y_start <= 69)
            {
                // 显示菜单
                PlayJpg(head_muen->next, 0, 0);
                break;
            }
        }
    }

    // 释放资源
    destroyList(head_photo);
    destroyList(head_exit);
    destroyList(head_muen);
    return;
}

void woodenFish(void)
{

    // 把菜单图片存入节点
    pNode head_muen = trajpgPic("./img/muen");
    // 把木鱼存入节点
    pNode head_flash = trajpgPic("./img/woodenflash");
    pNode tmp_flash = head_flash;
    // 把退出图标存入节点
    pNode head_exit = trajpgPic("./img/exit1");

    //  计数
    int Num;
    FILE *fp1 = fopen("./font/num.txt", "r+");
    if (fp1 == NULL)
    {
        perror("打开计数文件失败");
        return;
    }

    fscanf(fp1, "%d", &Num);

    // 木鱼
    printf("木鱼\n");
    PlayJpg(head_flash->next, 0, 0);

    PlayJpg(head_exit->next, 0, 730);
    woodenFishNum(Num);

    // 捕获
    int x_in, y_in, x_out, y_out;
    getTouchxy(&x_in, &y_in, &x_out, &y_out);
    // printf("按下坐标(%d,%d)\n", x_in, y_in);
    // printf("松手坐标(%d,%d)\n", x_out, y_out);

    // 退出
    if (x_in >= 730 && x_in <= 799 && y_in >= 0 && y_in <= 69)
    {
        // 显示菜单
        PlayJpg(head_muen->next, 0, 0);
        // 释放资源
        destroyList(head_flash);
        destroyList(head_exit);
        destroyList(head_muen);
        fseek(fp1, 0, SEEK_SET);
        fprintf(fp1, "%d", Num);
        fclose(fp1);

        return;
    }
    else
    {
        while (1)
        {
            // 退出循环条件
            // 退出
            if (x_in >= 730 && x_in <= 799 && y_in >= 0 && y_in <= 69)
            {
                // 显示菜单
                PlayJpg(head_muen->next, 0, 0);
                break;
            }
            else
            {
                PlayJpg(tmp_flash->next, 0, 0);
                tmp_flash = tmp_flash->next;
                PlayJpg(tmp_flash->next, 0, 0);
                PlayJpg(head_exit->next, 0, 730);
                Num++;
                fseek(fp1, 0, SEEK_SET);
                fprintf(fp1, "%d", Num);
                woodenFishNum(Num);
                if (tmp_flash != head_flash)
                {
                    tmp_flash = head_flash;
                }
            }

            getTouchxy(&x_in, &y_in, &x_out, &y_out);
        }
    }

    // 释放资源
    destroyList(head_flash);
    destroyList(head_exit);
    destroyList(head_muen);
    fseek(fp1, 0, SEEK_SET);
    fprintf(fp1, "%d", Num);
    fclose(fp1);
    return;
}

char *return_buf_next(int n)
{
    FILE *fp = fopen("./font/Novel.txt", "r");
    if (fp == NULL)
    {
        perror("打开小说文件失败");
        return NULL;
    }

    int k = 0;
    char buf1[MAX_BUF_SIZE] = {0};
    fseek(fp, n, SEEK_SET);
    for (int i = 0; i < 7; i++)
    {
        bzero(buf1, MAX_BUF_SIZE);
        fgets(buf1, MAX_BUF_SIZE, fp);
        // printf("长度：%zu\n", strlen(buf1));
        k += strlen(buf1);
    }
    fseek(fp, n, SEEK_SET);

    char *buf = calloc(k, sizeof(char));
    fread(buf, k, 1, fp);
    fclose(fp);

    return buf;
}

char *return_buf_prev(int n)
{
    FILE *fp = fopen("./font/Novel.txt", "r");
    if (fp == NULL)
    {
        perror("打开小说文件失败");
        return NULL;
    }

    int k = 0;
    char buf1[MAX_BUF_SIZE] = {0};
    fseek(fp, n, SEEK_SET);
    for (int i = 0; i < 7; i++)
    {
        bzero(buf1, MAX_BUF_SIZE);
        fgets(buf1, MAX_BUF_SIZE, fp);
        k += strlen(buf1);
    }
    fseek(fp, n, SEEK_SET);

    char *buf = calloc(k, sizeof(char));
    fread(buf, k, 1, fp);
    fclose(fp);

    return buf;
}

char *readnovel()
{
    FILE *fp = fopen("./font/Novel.txt", "r");
    if (fp == NULL)
    {
        perror("打开小说文件失败");
        return NULL;
    }

    int k = 0;
    char buf1[MAX_BUF_SIZE] = {0};
    for (int i = 0; i < 7; i++)
    {
        bzero(buf1, MAX_BUF_SIZE);
        fgets(buf1, MAX_BUF_SIZE, fp);
        // printf("长度：%zu\n", strlen(buf1));
        k += strlen(buf1);
    }

    char *buf = calloc(k, sizeof(char));
    fseek(fp, 0, SEEK_SET);
    fread(buf, k, 1, fp);
    fclose(fp);

    return buf;
}

void Novel(void)
{
    char *buf2 = NULL;
    // 把菜单图片存入节点
    pNode head_muen = trajpgPic("./img/muen");
    // 把退出图标存入节点
    pNode head_exit = trajpgPic("./img/exit1");

    buf2 = readnovel();
    displayNovel(buf2);

    int n = 0;
    PlayJpg(head_exit->next, 0, 730);
    int page = 0; // 当前页 // 记录页数
    int page_n[50] = {0};
    page_n[page] = n; // 记录每页的偏移量
    int x_start, y_start, x_end, y_end;
    getSwipexy(&x_start, &y_start, &x_end, &y_end);

    if (x_start >= 730 && x_start <= 799 && y_start >= 0 && y_start <= 69)
    {
        PlayJpg(head_muen->next, 0, 0);
        destroyList(head_exit);
        destroyList(head_muen);
        free(buf2);
        buf2 = NULL;
        return;
    }
    else
    {

        while (1)
        {
            if (x_start - x_end <= -10)
            {
                printf("上一页\n");
                page--;
                n = page_n[page];

                if (page < 0)
                {
                    printf("这是第一页\n");
                    n = 0;
                    page = 0;
                }
                else
                {
                    free(buf2);
                    buf2 = return_buf_prev(n);
                }

                displayNovel(buf2);
                PlayJpg(head_exit->next, 0, 730);
            }

            if (x_start - x_end >= 10)
            {

                if (strlen(buf2) != 0)
                {
                    printf("下一页\n");
                    n += strlen(buf2); // 记录总偏移量
                    page++;
                    page_n[page] = n; // 记录每页的偏移量
                    free(buf2);
                    buf2 = return_buf_next(n);
                    displayNovel(buf2);
                    PlayJpg(head_exit->next, 0, 730);
                }
                else
                {
                    //如果不想跳转到第一页，可以注释掉这里面的内容
                    printf("已经是最后一页了\n");
                    free(buf2);
                    n = 0;
                    page = 0;
                    buf2 = return_buf_next(n);
                    displayNovel(buf2);
                    PlayJpg(head_exit->next, 0, 730);
                }
            }

            // 退出循环条件
            getSwipexy(&x_start, &y_start, &x_end, &y_end);
            // 退出
            if (x_start >= 730 && x_start <= 799 && y_start >= 0 && y_start <= 69)
            {
                // 显示菜单
                PlayJpg(head_muen->next, 0, 0);

                break;
            }
        }
    }

    // 释放资源
    free(buf2);
    buf2 = NULL;
    destroyList(head_exit);
    destroyList(head_muen);
    return;
}
