#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include "jpgch.h"
#include "datatype.h"
#include "list.h"
#include "lcd.h"
#include "trajpgPic.h"
#include "jpeglib.h"
#include "muens.h"

#define LCD_W 800
#define LCD_H 480

int get_jpeg_dimensions(const char *filename, int *width, int *height)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);

    jpeg_read_header(&cinfo, TRUE);

    *width = cinfo.image_width;
    *height = cinfo.image_height;

    jpeg_destroy_decompress(&cinfo);

    fclose(fp);

    return 0;
}

pNode PlayJpg(pNode node, int y_s, int x_s)
{
    // pNode head = trajpgPic(jpgpath);

    // 打开JPG文件
    FILE *fp = fopen(node->data.name, "r");
    if (fp == NULL)
    {
        printf("Error opening file%d\n",__LINE__);
        perror("Error opening file");
        return NULL;
    }

    // 获取对应文件的大小
    struct stat statbuf;
    if (stat(node->data.name, &statbuf) != 0)
    {
        perror("Error getting file size");
        fclose(fp);
        return NULL;
    }

    // 申请一个堆内存，大小取决于JPG文件的实际大小
    char *jpgdata = calloc(statbuf.st_size, sizeof(char));
    if (jpgdata == NULL)
    {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // 读取JPG文件中的所有数据，并存入对应的堆内存中
    if (fread(jpgdata, statbuf.st_size, 1, fp) != 1)
    {
        perror("Error reading file");
        fclose(fp);
        free(jpgdata);
        return NULL;
    }

    // 获取 JPEG 图片的长和宽
    int width, height;
    if (get_jpeg_dimensions(node->data.name, &width, &height) != 0)
    {
        fclose(fp);
        return NULL;
    }

    // printf("width:%d\n",width);
    // printf("height:%d\n",height);

    // // 计算图像为了满足行宽为4的倍数补齐的字节数
    // int count = (4 - (width * 3 % 4))%4;

    fclose(fp); // 关闭文件，释放资源

    // 把jpgdata图像的原始数据转换为bgr颜色数据
    char *bgr = jpg2bgr(jpgdata, statbuf.st_size);
    free(jpgdata); // 释放原始数据内存

    // 初始化显示器
    LCDInit();

    // 把bgr数据转换为argb数据
    int argbBuf[height][width];
    bzero(argbBuf, sizeof(argbBuf));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            argbBuf[y][x] = bgr[(x + y * width) * 3 + 0] << 0 | // 蓝色
                            bgr[(x + y * width) * 3 + 1] << 8 | // 绿色
                            bgr[(x + y * width) * 3 + 2] << 16; // 红色
        }
    }

    // 写入到LCD的设备文件即可
    //  memcpy(map, argbBuf, 800 * 480 * 4);
    for (int y = y_s; y < height + y_s; y++)
    {
        for (int x = x_s; x < width + x_s; x++)
        {
            if (x < 800 && y < 480)
            {
                *(map + x + y * LCD_W) = argbBuf[y - y_s][x - x_s];
            }
        }
    }

    // printf("Write to LCD\n");

    // 释放argb数据
    LCDClose();
    return node->next;
}






// 刮刮乐
void Playcard(int y_s, int x_s)
{
    // 显示刮另一张
    // PlayJpg(head_again->next, 410, 660);

    // 把菜单图片存入节点
    pNode head_muen = trajpgPic("./img/muen");
    // 把刮刮乐卡片存入节点
    pNode head_scratchcard = trajpgPic("./img/scratchcard/card");
    // 把奖项图片存入节点
    pNode head_price = trajpgPic("./img/scratchcard/price");
    pNode tmp_price = head_price;
    // 显示刮另一张
    pNode head_again = trajpgPic("./img/scratchcard/ScrapeAnotherOne");

    // 把退出图标存入节点
    pNode head_exit = trajpgPic("./img/exit1");

    // 显示刮奖页面
    PlayJpg(head_scratchcard->next, 0, 0);
    PlayJpg(head_exit->next, 0, 730);
    // 显示刮另一张
    PlayJpg(head_again->next, 410, 660);

    while (1)
    {
        // 打开JPG文件
        FILE *fp = fopen(tmp_price->next->data.name, "r");
        if (fp == NULL)
        {
            perror("Error opening file");
            // printf("_%d_\n",__LINE__);
            return;
        }

        // 获取对应文件的大小
        struct stat statbuf;
        if (stat(tmp_price->next->data.name, &statbuf) != 0)
        {
            perror("Error getting file size");
            fclose(fp);
            return;
        }

        // 申请一个堆内存，大小取决于JPG文件的实际大小
        char *jpgdata = calloc(statbuf.st_size, sizeof(char));
        if (jpgdata == NULL)
        {
            perror("Memory allocation failed");
            fclose(fp);
            return;
        }

        // 读取JPG文件中的所有数据，并存入对应的堆内存中
        if (fread(jpgdata, statbuf.st_size, 1, fp) != 1)
        {
            perror("Error reading file");
            fclose(fp);
            free(jpgdata);
            return;
        }

        // 获取 JPEG 图片的长和宽
        int width, height;
        if (get_jpeg_dimensions(tmp_price->next->data.name, &width, &height) != 0)
        {
            fclose(fp);
            return;
        }

        // 把jpgdata图像的原始数据转换为bgr颜色数据
        char *bgr = jpg2bgr(jpgdata, statbuf.st_size);
        free(jpgdata); // 释放原始数据内存

        // 初始化显示器
        LCDInit();

        // 把bgr数据转换为argb数据
        int argbBuf[height][width];
        bzero(argbBuf, sizeof(argbBuf));

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                argbBuf[y][x] = bgr[(x + y * width) * 3 + 0] << 0 | // 蓝色
                                bgr[(x + y * width) * 3 + 1] << 8 | // 绿色
                                bgr[(x + y * width) * 3 + 2] << 16; // 红色
            }
        }

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
        int x_in, y_in;

        while (1)
        {

            while (1)
            {
                // getTouchxy(&x_in, &y_in, &x_out, &y_out);
                int ret_val = read(fd_ts, &tsEvent, sizeof(tsEvent));
                // printf("成功读取：%d字节数据..\n", ret_val);

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
                        x_in = x_tmp;
                        y_in = y_tmp;
                    }
                    if (x_in >= 660 && x_in < 800 && y_in < 480 && y_in >= 410)
                    {
                        break;
                    }
                    if (x_in >= 730 && x_in <= 799 && y_in >= 0 && y_in <= 69)
                    {
                        break;
                    }
                }

                int newX, newY;
                for (int i = 0; i < 50; i++)
                {
                    for (int j = 0; j < 50; j++)
                    {

                        newX = x_tmp + i;
                        newY = y_tmp + j;

                        // 检查坐标是否在屏幕范围内

                        if (newX >= 252 && newX < 517 && newY >= 255 && newY < 375)
                        {
                            *(map + newX + newY * LCD_W) = argbBuf[newY - y_s][newX - x_s];
                        }
                        newX = x_tmp - i;
                        newY = y_tmp + j;

                        // 检查坐标是否在屏幕范围内

                        if (newX >= 252 && newX < 517 && newY >= 255 && newY < 375)
                        {
                            *(map + newX + newY * LCD_W) = argbBuf[newY - y_s][newX - x_s];
                        }
                        newX = x_tmp + i;
                        newY = y_tmp - j;

                        // 检查坐标是否在屏幕范围内

                        if (newX >= 252 && newX < 517 && newY >= 255 && newY < 375)
                        {
                            *(map + newX + newY * LCD_W) = argbBuf[newY - y_s][newX - x_s];
                        }
                        newX = x_tmp - i;
                        newY = y_tmp - j;

                        // 检查坐标是否在屏幕范围内

                        if (newX >= 252 && newX < 517 && newY >= 255 && newY < 375)
                        {
                            *(map + newX + newY * LCD_W) = argbBuf[newY - y_s][newX - x_s];
                        }
                    }
                }
            }

            if (x_in >= 660 && x_in < 800 && y_in < 480 && y_in >= 410)
            {
                PlayJpg(head_scratchcard->next, 0, 0);
                PlayJpg(head_exit->next, 0, 730);
                // 显示刮另一张
                PlayJpg(head_again->next, 410, 660);
                tmp_price = tmp_price->next;
                if (tmp_price->next == head_price)
                {
                    tmp_price = tmp_price->next;
                }
                break;
            }

            if (x_in >= 730 && x_in <= 799 && y_tmp >= 0 && y_tmp <= 69)
            {
                // 显示首页菜单图片
                PlayJpg(head_muen->next, 0, 0);

                // 关闭触摸屏设备
                close(fd_ts);
                LCDClose();
                // 释放资源
                destroyList(head_scratchcard);
                destroyList(head_price);
                destroyList(head_exit);
                destroyList(head_muen);
                destroyList(head_again);

                return;
            }
        }

        // 关闭触摸屏设备
        close(fd_ts);
    }

    // 注意，要显示头节点有些要跳出循环，要不然会发生段错误（要不然不能打开JPG文件）

    return;
}