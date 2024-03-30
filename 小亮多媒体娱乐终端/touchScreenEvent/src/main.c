#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "PlayJpg.h"
#include "list.h"
#include "trajpgPic.h"
#include "datatype.h"
#include "muens.h"

// 运行命令 arm-linux-gcc ./src/*.c -o ./bin/demo -I ./inc/ -ljpeg -L./lib/ -lm -lfont -L./lib/

int main(int argc, const char *argv[])
{
    // 把菜单图片存入节点
    pNode head_muen = trajpgPic("./img/muen");

    // 显示首页菜单图片
    PlayJpg(head_muen->next, 0, 0);

    // 选择功能
    int x_in, y_in, x_out, y_out;

    while (1)
    {
        getTouchxy(&x_in, &y_in, &x_out, &y_out);

        printf("按下坐标(%d,%d)\n", x_in, y_in);
        printf("松手坐标(%d,%d)\n", x_out, y_out);

        // 根据坐标值进行相应处理

        // 播放相册
        if (x_in > 373 && x_in < 800 && y_in < 220 && y_in >= 0)
        {
            // 相册
            picture();
        }

        // 木鱼
        if (x_in > 0 && x_in <= 373 && y_in > 220 && y_in <= 800)
        {
           
            // 木鱼
            woodenFish();

        }

        // 刮刮乐
        if (x_in > 0 && x_in <= 373 && y_in < 220 && y_in >= 0)
        {
            // 刮刮乐
            Playcard(255, 252);
        }

        // 小说
        if (x_in > 373 && x_in <= 800 && y_in > 220 && y_in <= 480)
        {
            Novel();
        }
    }

    // 释放资源
    destroyList(head_muen);

    return 0;
}