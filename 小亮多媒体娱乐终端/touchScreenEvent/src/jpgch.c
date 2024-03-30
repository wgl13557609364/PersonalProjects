#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include "jpeglib.h"
#include <dirent.h>
#include "datatype.h"
#include "list.h"



// 参数说明：
//   jpgdata: jpg图片数据
//   jpgsize: jpg图片大小
// 返回值说明：
//   成功：指向rgb数据的指针
//   失败：NULL
char *jpg2bgr(const char *jpgdata, int jpgsize)
{
    // 1，声明解码结构体，以及错误管理结构体
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // 2，使用缺省的出错处理来初始化解码结构体
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // 3，配置该cinfo，使其从 jpgdata 中读取jpgsize个字节
    //    这些数据必须是完整的JPEG数据
    jpeg_mem_src(&cinfo, jpgdata, jpgsize);

    // 4，读取JPEG文件的头，并判断其格式是否合法
    if(!jpeg_read_header(&cinfo, true))
    {
        fprintf(stderr, "jpeg_read_header failed: "
            "%s\n", strerror(errno));
        return NULL;
    }

    // 5，开始解码
    jpeg_start_decompress(&cinfo);

    // 6，获取图片的尺寸信息
    // printf("宽：  %d\n", cinfo.output_width);
    // printf("高：  %d\n", cinfo.output_height);
    // printf("色深：%d\n", cinfo.output_components);

    // int row_stride = cinfo.width * cinfo.pixel_size;

    // 7，根据图片的尺寸大小，分配一块相应的内存bgrdata
    //    用来存放从jpgdata解码出来的图像数据
    unsigned long linesize = cinfo.output_width * cinfo.output_components;  // 计算一行中有多少字节
    unsigned long rgbsize  = linesize * cinfo.output_height;  // 计算所有像素点的大小
    char *bgrdata = calloc(1, rgbsize);  // 根据实际的BGR颜色数量以及大小来申请具体的BGR空间大小

    // 8，循环地将图片的每一行读出并解码到rgb_buffer中
    int line = 0;
    while(cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *buffer_array[1];
        buffer_array[0] = bgrdata + cinfo.output_scanline * linesize;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    // 9，解码完了，将jpeg相关的资源释放掉
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return bgrdata;
}



