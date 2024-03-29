#include "font.h"
#include <string.h>
#include "muens.h"
#define PAGE_WIDTH 800
#define LINE_HEIGHT 30

// 初始化Lcd
struct LcdDevice *init_lcd(const char *device)
{
	// 申请空间
	struct LcdDevice *lcd = malloc(sizeof(struct LcdDevice));
	if (lcd == NULL)
	{
		return NULL;
	}

	// 1打开设备
	lcd->fd = open(device, O_RDWR);
	if (lcd->fd < 0)
	{
		perror("open lcd fail");
		free(lcd);
		return NULL;
	}

	// 映射
	lcd->mp = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd->fd, 0);

	return lcd;
}

int woodenFishNum(int num)
{

	// 初始化Lcd
	struct LcdDevice *lcd = init_lcd("/dev/fb0");

	// 打开字体
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");

	// 字体大小的设置
	fontSetSize(f, 58);

	// 创建一个画板（点阵图）
	bitmap *bm = createBitmapWithInit(350, 100, 4, getColor(0, 0, 0, 0)); // 也可使用createBitmapWithInit函数，改变画板颜色
	// bitmap *bm = createBitmap(288, 100, 4);

	char buf[512] = {0};
	sprintf(buf, "%s%d", "功德+", num);

	// 将字体写到点阵图上
	fontPrint(f, bm, 5, 0, buf, getColor(0, 0, 255, 0), 0);

	// 把字体框输出到LCD屏幕上
	show_font_to_lcd(lcd->mp, 0, 0, bm);

	// 把字体框输出到LCD屏幕上
	//  show_font_to_lcd(lcd->mp,200,200,bm);

	// 关闭字体，关闭画板
	fontUnload(f);
	destroyBitmap(bm);
	return num;
}



void displayNovel(char *buf)
{
	

	// 初始化Lcd
	struct LcdDevice *lcd = init_lcd("/dev/fb0");

	// 打开字体
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");

	// 字体大小的设置
	fontSetSize(f, 70);

	// 创建一个画板（点阵图）
	bitmap *bm = createBitmapWithInit(800, 480, 4, getColor(0,153, 217, 153)); // 也可使用createBitmapWithInit函数，改变画板颜色
	// bitmap *bm = createBitmap(288, 100, 4);


	// 将字体写到点阵图上
	fontPrint(f, bm, 5, 0, buf, getColor(0, 0, 0, 0), 0);
	// 把字体框输出到LCD屏幕上
	show_font_to_lcd(lcd->mp, 0, 0, bm);

	// 把字体框输出到LCD屏幕上
	//  show_font_to_lcd(lcd->mp,200,200,bm);

	// 关闭字体，关闭画板
	fontUnload(f);
	destroyBitmap(bm);
	return ;
}

