#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


#define LCD_PATH "/dev/fb0"
#define LCD_W    800
#define LCD_H    480
#define LCD_SIZE ((LCD_H)*(LCD_W)*4)

int fd_lcd;
int * map;

void LCDInit( void )
{
    // - 打开LCD并初始化（MMAP）
    fd_lcd = open( LCD_PATH , O_RDWR );
    if (fd_lcd == -1 )
    {
        perror("open fb0 file error");
        return  ;
    }
    
    // 内存映射
    map = mmap( NULL , LCD_SIZE , PROT_READ | PROT_WRITE , MAP_SHARED , fd_lcd , 0  );
    if (map == MAP_FAILED)
    {
        perror("mmap error ");
        return ;
    }
    return ;
}

void LCDClose(void)
{
    munmap(map ,LCD_SIZE );
    close(fd_lcd);
}