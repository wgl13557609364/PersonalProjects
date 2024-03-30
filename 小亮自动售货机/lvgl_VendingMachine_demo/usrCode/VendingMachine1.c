#include "../lvgl/lvgl.h"
// #include "../lvgl/examples/lv_examples.h"
// #include "dir.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

// 注意，频繁使用lvgl库频繁修改标签时出现错误，会导致段错误，这跟lvgl的渲染有关，

// 售货机商品结构体
typedef struct
{
    char name[50];
    float price;
    char icon_path[128]; // 商品图标地址
    int stock;           // 商品库存数量
} VendingMachineItem;

// 购物车商品结构体
typedef struct
{
    VendingMachineItem * item; // 商品类型
    int quantity;              // 购买的商品数量
} ShoppingCartItem;

// 购物车结构体
typedef struct
{
    ShoppingCartItem items[15]; // 最多可放15个商品
    int count;                  // 购物车商品种类数量
} ShoppingCart;

// 按钮管理结构体
typedef struct
{
    lv_obj_t * btn;
    VendingMachineItem * item;
    lv_obj_t * btn_label;
} ShoppingCarBtnManager;

// 售货机商品列表
// VendingMachineItem machineItems[] = {
//     // 图片长和宽最好是8的倍数，要不然解码能力降低，造成卡顿
//     {"kele", 1.99, "S:/pan/xiaoliang/img/commodity/1-coke.jpg", 5},
//     {"weita", 2.49, "S:/pan/xiaoliang/img/commodity/10-Vitamin_water.jpg", 3},
//     {"chapi", 0.99, "S:/pan/xiaoliang/img/commodity/11-TeaPi.jpg", 7},
//     {"fenda", 3.99, "S:/pan/xiaoliang/img/commodity/2-Fanta.jpg", 2},
//     {"molicha", 3.56, "S:/pan/xiaoliang/img/commodity/3-JasmineHoneyTea.jpg", 8},
//     {"water", 4.99, "S:/pan/xiaoliang/img/commodity/4-SpringWater.jpg", 4},
//     {"binghoncha", 2.99, "S:/pan/xiaoliang/img/commodity/5-IcedBlackTea.jpg", 6},
//     {"maidong", 1.49, "S:/pan/xiaoliang/img/commodity/7-Pulsation.jpg", 10},
//     {"ADgai", 1.99, "S:/pan/xiaoliang/img/commodity/6-ADCalciumMilk.jpg", 9},
//     {"wangwang", 2.49, "S:/pan/xiaoliang/img/commodity/8-WantWantChildren'smilk.jpg", 5}

//     // 添加更多商品
// };

VendingMachineItem machineItems[50] = {0};           // 最多商品种类数量
int machineItems_index              = 0;             // 商品种类下标
lv_obj_t * cart_list                = NULL;          // 购物车商品列表对象
lv_obj_t * SelectList               = NULL;          // 商品选择列表对象
lv_obj_t * total_label              = NULL;          // 商品总结算金额标签对象
ShoppingCart cart;                                   // 购物车对象
lv_obj_t * msgbox                            = NULL; // 全局变量，用于存储消息框对象
ShoppingCarBtnManager carListBtn_manager[15] = {0};  // 按钮管理数组
int carListBtn_index                         = 0;    // 管理数量（下标）
int tcp_socket                               = -1;
int OrderId                                  = 1;    // 订单编号
lv_obj_t * playbackPanel                     = NULL; // 广告面板
char * advcpath                              = "S:/pan/xiaoliang/img/gif/guanggao1.gif";
lv_obj_t * img                               = NULL;

// 页面管理结构体
struct page
{
    char name[50];       // 页面名
    int page_level;      // 页面等级，依靠这个来实现对上一页面的切换，该值越小，等级越高
    lv_obj_t * body_obj; // 页面结构体
    void * private_data; // 私有数据
    void (*onInit)(struct page * page);                             // 创建页面函数
    void (*onExit)(struct page * old_page, struct page * new_page); // 退出页面函数
};

// 页面管理其他相关变量
static struct page page_admin[50]; // 存储所有创建好的页面
static int page_num = 0;           // 记录 page_admin[] 数组中存在多少个页面
struct page * cur_page;            // 记录当前显示的页面

// 查找页面
static struct page * page_search(char * name)
{
    for(int i = 0; i < page_num; i++) {
        if(strcmp(page_admin[i].name, name) == 0) return &page_admin[i];
    }
    return NULL;
}

/**
 * @brief 页面切换函数
 * @param new_page_name 新页面名
 * @return 0 成功     -1 新页面不存在
 */
int page_transition(char * new_page_name)
{
    printf("transition page = %s\n", new_page_name);
    if(strcmp(new_page_name, "") == 0 || new_page_name == NULL) return -1;

    struct page * new_page = page_search(new_page_name);
    if(new_page != NULL) {
        cur_page->onExit(cur_page, new_page);
        new_page->onInit(new_page);
        cur_page = new_page;
        return 0;
    } else {
        return -1;
    }
}

/**
 * @brief 返回上一页面
 * @param page 当前页面
 * @return struct page 指针 上一页面    NULL 失败
 */
struct page * return_prev_page(struct page * page)
{
    if(page == NULL) return NULL;

    int cur_page_level = page->page_level;
    for(int i = page_num - 1; i >= 0; i--) {
        if(page_admin[i].page_level < cur_page_level) {
            return &page_admin[i];
        }
    }
    return NULL;
}

/* 创建一个指定样式的空白屏幕 */
lv_obj_t * create_new_screen(void)
{
    lv_obj_t * main_obj = lv_obj_create(NULL);
    // lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_clean(main_obj);
    lv_obj_set_size(main_obj, 800, 480); // 根据个人屏幕大小修改
    // lv_obj_add_style(main_obj, &page_style, 0);

    return main_obj;
}

// 更新商品总结算金额
void update_total_label()
{
    float total = 0.0;
    for(int i = 0; i < cart.count; i++) {
        total += cart.items[i].item->price * cart.items[i].quantity;
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "TotalPrice:%.2f", total);
    lv_label_set_text(total_label, buf);
}

// 添加商品到购物车
void add_item_to_cart(VendingMachineItem * item)
{
    if(cart.count > 13) {
        // 购物车已满，无法添加更多商品
        printf("购物车已满,无法添加更多商品！\n");
        return;
    }

    // 检查购物车中是否已有相同商品
    for(int i = 0; i < cart.count; i++) {
        if(cart.items[i].item == item) {
            // 商品已存在购物车中
            if(cart.items[i].quantity < item->stock) {
                // 增加数量
                cart.items[i].quantity++;
                // 查找管理结构体
                for(int j = 0; j < carListBtn_index; j++) {
                    if(carListBtn_manager[j].item == item) {

                        // 更新购物车列表显示
                        char buf[128] = {0};
                        snprintf(buf, sizeof(buf), "%s(%d/%d)\nprice:%.2f", item->name, cart.items[i].quantity,
                                 item->stock, item->price * cart.items[i].quantity);
                        // lv_obj_clean(btn_manager[j].btn);
                        lv_obj_del(carListBtn_manager[j].btn);
                        carListBtn_manager[j].btn = lv_list_add_btn(cart_list, item->icon_path, buf);
                        // update_total_label();
                        break;
                    }
                }

                // lv_list_add_btn(cart_list, item->icon_path, buf);
                // lv_obj_del;
                // lv_obj_get_index;
                // lv_obj_move_to_index;
            }
            update_total_label(); // 更新商品总结算金额
            return;
        }
    }

    // 商品未在购物车中，则添加新的购物车商品
    if(item->stock > 0) {
        ShoppingCartItem * cartItem = &cart.items[cart.count];
        cartItem->item              = item;
        cartItem->quantity          = 1;
        cart.count++;

        // 更新购物车列表显示
        char buf[128] = {0};
        snprintf(buf, sizeof(buf), "%s(1/%d)\nprice:%.2f", item->name, item->stock, item->price);
        lv_obj_t * btn                            = lv_list_add_btn(cart_list, item->icon_path, buf);
        carListBtn_manager[carListBtn_index].btn  = btn;
        carListBtn_manager[carListBtn_index].item = item;
        carListBtn_index++;

        update_total_label(); // 更新商品总结算金额
    }
}

// 添加商品按钮回调函数
void addGoodsButton_event_handler(lv_event_t * event)
{
    lv_event_code_t code      = lv_event_get_code(event);
    VendingMachineItem * item = lv_event_get_user_data(event);
    if(code == LV_EVENT_CLICKED) {
        // printf("按钮被点击\n");
        // 在这里处理按钮点击事件的逻辑
        add_item_to_cart(item);
    }
}

// 结算对话框回调函数
void msgbox_confirm_event_handler(lv_event_t * event)
{
    lv_obj_t * obj        = lv_event_get_current_target(event);
    const char * btn_text = lv_msgbox_get_active_btn_text(obj);
    if(strcmp(btn_text, "Confirm") == 0) {
        // 在此处处理确认按钮点击事件的逻辑
        // printf("确认按钮被点击\n");
        lv_obj_del(msgbox);            // 删除消息对话框
        msgbox                 = NULL; // 将消息对话框指针重置为NULL
        struct page * new_page = &page_admin[1];
        new_page->onInit(new_page);
        cur_page->onExit(cur_page, new_page);
        cur_page = new_page;
    } else if(strcmp(btn_text, "Cancel") == 0) {
        // printf("取消按钮被点击\n");
        lv_obj_del(msgbox); // 删除消息对话框
        msgbox = NULL;      // 将消息对话框指针重置为NULL
    }
}

// SettlementButton回调函数
static void SettlementButton_event_cb(lv_event_t * event)
{

    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_CLICKED) {
        if(cart.count < 1) {
            printf("购物车没有商品，请添加商品！\n");
            return;
        }
        static const char * btns_txts[] = {"Cancel", "Confirm", ""};
        msgbox = lv_msgbox_create(lv_scr_act(), "SettlementPage", "Are you sure you want to settle the bill?",
                                  btns_txts, true);
        lv_obj_set_width(msgbox, lv_disp_get_hor_res(NULL) / 2);
        // lv_obj_align(msgbox, LV_ALIGN_CENTER, 0, 0);
        lv_obj_center(msgbox);
        lv_obj_add_event_cb(msgbox, msgbox_confirm_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

// 发送订单函数
void sendOrderToServer()
{

    // 获取当前时间的时间戳
    time_t currentTime;
    time(&currentTime);

    // 将时间戳转换为本地时间
    struct tm * localTime = localtime(&currentTime);

    // 定义格式化字符串
    char dateTimeFormat[20];
    strftime(dateTimeFormat, sizeof(dateTimeFormat), "%Y-%m-%d %H:%M:%S", localTime);
    // 打开订单文件来存储订单
    FILE * fp = fopen("order.txt", "w+");

    for(int i = 0; i < cart.count; i++) {

        fprintf(fp, "订单编号:%d 时间:%s 商品名:%s 单价:%.2f 数量:%d 总价:%.2f\n", OrderId++, dateTimeFormat,
                cart.items[i].item->name, cart.items[i].item->price, cart.items[i].quantity,
                cart.items[i].item->price * cart.items[i].quantity);
    }
    fseek(fp, 0, SEEK_END);
    // 获取文件大小
    int fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // 分配足够的内存来存储整个文件内容
    char * buffer = calloc(fileSize + 1, sizeof(char));
    if(buffer == NULL) {
        perror("内存分配失败");
        fclose(fp);
        return;
    }

    // 一次性读取整个文件内容到缓冲区中
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, fp);
    if(bytesRead != fileSize) {
        perror("读取文件失败");
        free(buffer);
        fclose(fp);
        return;
    }

    // 发送给服务器
    buffer[fileSize] = '\0'; // 添加字符串结束符
    if(write(tcp_socket, buffer, fileSize) > 0) {
        printf("发送成功\n");
    } else {
        printf("发送失败\n");
    }
    // 清理资源
    free(buffer);
    fclose(fp);
    return;
}

// 查看商品信息函数
void viewItemInfo()
{
    if (machineItems_index <1)
    {
        printf("没有商品信息\n");
        return;
    }
    
    // 打开商品文件来存储商品信息
    FILE * fp = fopen("CommodityInformation.txt", "w+");
    for(int i = 0; i < machineItems_index; i++) {
        fprintf(fp, "【商品详情】商品名:%s 单价:%.2f 剩余库存:%d\n", machineItems[i].name, machineItems[i].price,
                machineItems[i].stock);
    }
    fseek(fp, 0, SEEK_END);
    // 获取文件大小
    int fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // 分配足够的内存来存储整个文件内容
    char * buffer = calloc(fileSize + 1, sizeof(char));
    if(buffer == NULL) {
        perror("内存分配失败");
        fclose(fp);
        return;
    }

    // 一次性读取整个文件内容到缓冲区中
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, fp);
    if(bytesRead != fileSize) {
        perror("读取文件失败");
        free(buffer);
        fclose(fp);
        return;
    }

    // 发送给服务器
    buffer[fileSize] = '\0'; // 添加字符串结束符
    if(write(tcp_socket, buffer, fileSize) > 0) {
        printf("发送成功\n");
    } else {
        printf("发送失败\n");
    }
    // 清理资源
    free(buffer);
    fclose(fp);
    return;
}

// 支付成功回调函数
static void Pay_event_cb(lv_event_t * event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_CLICKED) {
        // 发送订单给服务器
        sendOrderToServer();
        // // 清空购物车列表
        // lv_obj_clean(cart_list);
        // 清空购物车按钮管理结构体数组
        memset(carListBtn_manager, 0, sizeof(carListBtn_manager));
        // 清空物车按钮管理结构体下标
        carListBtn_index = 0;
        // 修改商品库存数量
        for(int i = 0; i < cart.count; i++) {
            cart.items[i].item->stock -= cart.items[i].quantity;
            // printf("商品名称：%s，商品库存：%d\n", cart.items[i].item->name, cart.items[i].item->stock);
        }
        // 清空购物车商品
        memset(&cart, 0, sizeof(cart));
        // cart.count             = 0;

        // 返回主页
        struct page * new_page = &page_admin[0];
        new_page->onInit(new_page);
        cur_page->onExit(cur_page, new_page);
        cur_page = new_page;
        // 添加商品到商品列表
        for(int i = 0; i < machineItems_index; i++) {
            VendingMachineItem * item = &machineItems[i];
            char tmp1[128]            = {0};
            snprintf(tmp1, sizeof(tmp1), "%s\n%.2f$\nstock:%d\nAddToCar", item->name, item->price, item->stock);
            lv_obj_t * btn = lv_list_add_btn(SelectList, item->icon_path, tmp1);
            lv_obj_set_size(btn, 250, 120);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xF08080), 0);
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_style_pad_top(btn, 4, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0xF08080), 0);
            lv_obj_add_event_cb(btn, addGoodsButton_event_handler, LV_EVENT_ALL, item);
        }
    }
}

/*取消支付回调函数*/
static void cancelPay_event_cb(lv_event_t * event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_CLICKED) {
        // // 清空购物车列表
        // lv_obj_clean(cart_list);
        // 清空购物车按钮管理结构体数组
        memset(carListBtn_manager, 0, sizeof(carListBtn_manager));
        // 清空物车按钮管理结构体下标
        carListBtn_index = 0;
        // 清空购物车商品
        memset(&cart, 0, sizeof(cart));
        // 回到主页
        struct page * new_page = &page_admin[0];
        new_page->onInit(new_page);
        cur_page->onExit(cur_page, new_page);
        cur_page = new_page;
        // 添加商品到商品列表
        for(int i = 0; i < machineItems_index; i++) {
            VendingMachineItem * item = &machineItems[i];
            char tmp1[128]            = {0};
            snprintf(tmp1, sizeof(tmp1), "%s\n%.2f$\nstock:%d\nAddToCar", item->name, item->price, item->stock);
            lv_obj_t * btn = lv_list_add_btn(SelectList, item->icon_path, tmp1);
            lv_obj_set_size(btn, 250, 120);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xF08080), 0);
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_style_pad_top(btn, 4, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0xF08080), 0);
            lv_obj_add_event_cb(btn, addGoodsButton_event_handler, LV_EVENT_ALL, item);
        }
    }
}

static void EmptyCart_event_cb(lv_event_t * event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_CLICKED) {
        // 清空购物车列表
        lv_obj_clean(cart_list);
        // 清空购物车按钮管理结构体数组
        memset(carListBtn_manager, 0, sizeof(carListBtn_manager));
        // 清空物车按钮管理结构体下标
        carListBtn_index = 0;
        // 清空购物车商品
        memset(&cart, 0, sizeof(cart));
        // 更新价格为0
        lv_label_set_text(total_label, "TotalPrice:0.00");
    }
}

/*售货机主页面*/
void Vending_machine_home_page_init(struct page * page)
{

    lv_obj_t * body_obj = page->body_obj;
    lv_obj_set_style_bg_color(body_obj, lv_color_hex(0xFFB6C1), 0);

    /****************************商品广告和选择界面**************************************/
    lv_obj_t * machine = lv_obj_create(body_obj);
    lv_obj_set_style_bg_color(machine, lv_color_hex(0xFF7F50), 0);
    /*设置边框*/
    lv_obj_set_style_border_width(machine, 4, 0);
    lv_obj_set_style_border_color(machine, lv_color_hex(0xFFD700), 0); // 边框颜色
    lv_obj_set_style_radius(machine, 0, 0);                            // 四角弧度
    lv_obj_set_size(machine, 428, 480);
    lv_obj_set_style_pad_all(machine, 0, 0);
    lv_obj_align(machine, LV_ALIGN_LEFT_MID, 0, 0);

    /*logo位置*/
    LV_IMG_DECLARE(img_wink_png);
    lv_obj_t * img_logo;
    img_logo = lv_img_create(machine);
    lv_img_set_src(img_logo, &img_wink_png);
    lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 0);

    img_logo = lv_img_create(machine);
    /* Assuming a File system is attached to letter 'A'
     * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
    lv_img_set_src(img_logo, "S:/pan/xiaoliang/img/logo.png");
    lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 0);

    /************************************广告界面*****************************************/

    playbackPanel = lv_obj_create(machine);
    // 清除对象的样式
    lv_obj_remove_style_all(playbackPanel);
    /*设置边框*/
    lv_obj_set_style_border_width(playbackPanel, 0, 0);
    lv_obj_set_style_border_color(playbackPanel, lv_color_hex(0x000000), 0); // 边框颜色
    lv_obj_set_style_radius(playbackPanel, 0, 0);                            // 四角弧度
    lv_obj_set_style_pad_all(playbackPanel, 0, 0);

    lv_obj_set_style_opa(playbackPanel, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(playbackPanel, lv_color_hex(0x000000), 0);
    lv_obj_set_size(playbackPanel, 420, 240);
    lv_obj_align(playbackPanel, LV_ALIGN_TOP_MID, 0, 104);

    // LV_IMG_DECLARE(img_bulb_gif);
    // img = lv_gif_create(playbackPanel);
    // lv_gif_set_src(img, &img_bulb_gif);
    // lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    img = lv_gif_create(playbackPanel);
    /* Assuming a File system is attached to letter 'A'
     * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
    lv_gif_set_src(img, advcpath);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(img, 420);
    lv_obj_set_height(img, 240);

    /*******************************商品选择界面**********************************/
    SelectList = lv_list_create(machine);
    lv_obj_set_style_bg_color(SelectList, lv_color_hex(0xF08080), 0); // 背景
    lv_obj_set_style_radius(SelectList, 0, 0);                        // 四角弧度
    lv_obj_set_size(SelectList, 420, 128);                            // 边框默认宽度是2像素
    lv_obj_align(SelectList, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(SelectList, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(SelectList, 0, 0);
    /*设置边框*/
    lv_obj_set_style_border_width(SelectList, 4, 0); // 边框大小
    lv_obj_set_style_border_color(SelectList, lv_color_hex(0xFFD700), 0);
    // 设置顶部边框
    lv_obj_set_style_border_side(SelectList, LV_BORDER_SIDE_TOP, 0);

    /**************************************************************************************/

    /**************************************购物车界面***************************************/
    LV_IMG_DECLARE(img_wink_png);
    lv_obj_t * ShoppingCart;
    ShoppingCart = lv_img_create(body_obj);
    lv_img_set_src(ShoppingCart, &img_wink_png);
    lv_obj_align(ShoppingCart, LV_ALIGN_RIGHT_MID, 0, 0);

    ShoppingCart = lv_img_create(body_obj);
    /* Assuming a File system is attached to letter 'A'
     * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
    lv_img_set_src(ShoppingCart, "S:/pan/xiaoliang/img/CartPgColor.png");
    lv_obj_align(ShoppingCart, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(ShoppingCart, lv_color_hex(0xFF7F50), 0); // 背景颜色
    /*设置边框*/
    lv_obj_set_style_border_width(ShoppingCart, 4, 0);
    lv_obj_set_style_border_color(ShoppingCart, lv_color_hex(0x1E90FF), 0); // 边框颜色
    lv_obj_set_style_radius(ShoppingCart, 0, 0);                            // 四角弧度
    lv_obj_set_style_pad_all(ShoppingCart, 0, 0);

    /*商品列表*/
    cart_list = lv_list_create(ShoppingCart);
    lv_obj_set_size(cart_list, 364, 310);
    lv_obj_align(cart_list, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_pad_all(cart_list, 0, 0);
    lv_obj_set_style_bg_color(cart_list, lv_color_hex(0xa9e59a), 0); // 背景
    lv_obj_set_style_radius(cart_list, 0, 0);                        // 四角弧度
    lv_obj_set_style_pad_all(cart_list, 0, 0);
    /*设置边框*/
    lv_obj_set_style_border_width(cart_list, 0, 0); // 边框大小

    // 结算按钮
    lv_obj_t * SettlementButton = lv_btn_create(ShoppingCart);
    lv_obj_set_width(SettlementButton, 85);
    lv_obj_set_height(SettlementButton, 40);
    lv_obj_set_style_bg_color(SettlementButton, lv_color_hex(0x6A5ACD), 0); // 背景
    lv_obj_align(SettlementButton, LV_ALIGN_BOTTOM_RIGHT, -45, -30);
    lv_obj_add_event_cb(SettlementButton, SettlementButton_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t * checkout_label = lv_label_create(SettlementButton);
    lv_label_set_text(checkout_label, "Settlement");
    lv_obj_align(checkout_label, LV_ALIGN_CENTER, 0, 0);

    // 清空购物车按钮
    lv_obj_t * EmptyCartBtn = lv_btn_create(ShoppingCart);
    lv_obj_set_width(EmptyCartBtn, 85);
    lv_obj_set_height(EmptyCartBtn, 40);
    lv_obj_set_style_bg_color(EmptyCartBtn, lv_color_hex(0x6A5ACD), 0); // 背景
    lv_obj_align(EmptyCartBtn, LV_ALIGN_BOTTOM_LEFT, 45, -30);
    lv_obj_add_event_cb(EmptyCartBtn, EmptyCart_event_cb, LV_EVENT_ALL, NULL);
    checkout_label = lv_label_create(EmptyCartBtn);
    lv_label_set_text(checkout_label, "EmptyCart");
    lv_obj_align(checkout_label, LV_ALIGN_CENTER, 0, 0);
    total_label = lv_label_create(ShoppingCart);

    // 合计价钱
    lv_label_set_text(total_label, "TotalPrice:0.00");
    lv_obj_align_to(total_label, cart_list, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    /**************************************************************************************/
}

/*结算页面*/
void settlementPage_init(struct page * page)
{
    lv_obj_t * body_obj = page->body_obj;
    lv_obj_set_style_bg_color(body_obj, lv_color_hex(0xFFB6C1), 0);

    // 微信支付
    lv_obj_t * weixinQrcodeBtn = lv_btn_create(body_obj);
    /* 取消按钮所有的样式属性 */
    lv_obj_remove_style_all(weixinQrcodeBtn);
    // 设置按下状态的透明度为默认状态的透明度 /* 设置控件透明度 */
    lv_obj_set_style_bg_opa(weixinQrcodeBtn, 255, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(weixinQrcodeBtn, 255, LV_STATE_DEFAULT); // 默认
    lv_obj_set_size(weixinQrcodeBtn, 257, 350);
    lv_obj_align(weixinQrcodeBtn, LV_ALIGN_TOP_RIGHT, -100, 10);
    /*添加收款码*/
    LV_IMG_DECLARE(img_wink_png);
    lv_obj_t * img_weixin;
    img_weixin = lv_img_create(weixinQrcodeBtn);
    lv_img_set_src(img_weixin, &img_wink_png);
    lv_obj_align(img_weixin, LV_ALIGN_TOP_MID, 0, 0);
    img_weixin = lv_img_create(weixinQrcodeBtn);
    /* Assuming a File system is attached to letter 'A'
     * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
    lv_img_set_src(img_weixin, "S:/pan/xiaoliang/img/CollectionCode/weixinshoukuanma.png");
    lv_obj_align(img_weixin, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(weixinQrcodeBtn, Pay_event_cb, LV_EVENT_ALL, NULL);

    // 支付宝支付
    lv_obj_t * zhifubaoQrcodeBtn = lv_btn_create(body_obj);
    /* 取消按钮所有的样式属性 */
    lv_obj_remove_style_all(zhifubaoQrcodeBtn);
    // 设置按下状态的透明度为默认状态的透明度 /* 设置控件透明度 */
    lv_obj_set_style_bg_opa(zhifubaoQrcodeBtn, 255, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(zhifubaoQrcodeBtn, 255, LV_STATE_DEFAULT); // 默认
    lv_obj_set_size(zhifubaoQrcodeBtn, 257, 350);
    lv_obj_align(zhifubaoQrcodeBtn, LV_ALIGN_TOP_LEFT, 100, 10);
    /*添加收款码*/
    LV_IMG_DECLARE(img_wink_png);
    lv_obj_t * img_zhifubao;
    img_zhifubao = lv_img_create(zhifubaoQrcodeBtn);
    lv_img_set_src(img_zhifubao, &img_wink_png);
    lv_obj_align(img_zhifubao, LV_ALIGN_TOP_MID, 0, 0);
    img_zhifubao = lv_img_create(zhifubaoQrcodeBtn);
    /* Assuming a File system is attached to letter 'A'
     * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
    lv_img_set_src(img_zhifubao, "S:/pan/xiaoliang/img/CollectionCode/zhifubaoshoukma.jpg");
    lv_obj_align(img_zhifubao, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(zhifubaoQrcodeBtn, Pay_event_cb, LV_EVENT_ALL, NULL);

    // 取消支付按钮
    lv_obj_t * cancelPayBtn = lv_btn_create(body_obj);
    lv_obj_set_size(cancelPayBtn, 100, 60);
    lv_obj_align(cancelPayBtn, LV_ALIGN_BOTTOM_MID, 0, -30);
    // lv_obj_set_style_radius(cancelPayBtn, 0, 0);
    lv_obj_set_style_bg_color(cancelPayBtn, lv_color_hex(0xEE8262), 0);
    lv_obj_t * cancelPay_label = lv_label_create(cancelPayBtn);
    lv_label_set_text(cancelPay_label, "cancelPay");
    lv_obj_align(cancelPay_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(cancelPayBtn, cancelPay_event_cb, LV_EVENT_ALL, NULL);
}

/*主页面退出函数*/
void main_page_exit(struct page * old_page, struct page * new_page)
{
    lv_obj_clean(old_page->body_obj);
    lv_scr_load_anim(new_page->body_obj, LV_SCR_LOAD_ANIM_MOVE_TOP, 100, 0, false);
}

/*支付页面退出函数*/
void pay_page_exit(struct page * old_page, struct page * new_page)
{
    lv_obj_clean(old_page->body_obj);
    lv_scr_load_anim(new_page->body_obj, LV_SCR_LOAD_ANIM_MOVE_TOP, 100, 0, false);
}

void * tcp_cilent_thread_func(void * arg)
{

    // 1.创建TCP 通信对象
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 2.连接TCP 服务器
    struct sockaddr_in server_address;
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(8888);
    server_address.sin_addr.s_addr = inet_addr("111.230.200.178");
    int ret                        = connect(tcp_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    if(ret < 0) {
        perror("连接服务器失败\n");
        return NULL;
    } else {
        printf("连接服务器成功\n");
    }

    // 接收数据
    while(1) {
        char * recvbuf1 = calloc(1024 * 1024, sizeof(char));
        read(tcp_socket, recvbuf1, 1024 * 1024);
        if(strncmp(recvbuf1, "modify", 6) == 0) {
            // 打开货架商品列表文件
            FILE * SelectList_fp = fopen("SelectList.txt", "w+");
            if(SelectList_fp == NULL) {
                printf("打开货架商品列表文件失败\n");
            } else {
                printf("正在读取商品.......\n");
                // 读取要上架的商品
                while(1) {
                    if(0 <= fwrite(recvbuf1, 1024 * 1024, 1, SelectList_fp)) {
                        printf("读取商品完成.......\n");
                        break;
                    }
                }

                // 把文件指针指向初始位置
                // fseek(SelectList_fp, 7, SEEK_SET);
                fseek(SelectList_fp, 0, SEEK_SET);

                // 置零商品信息结构体
                memset(machineItems, 0, sizeof(machineItems));

                // 解析商品信息
                machineItems_index = 0;
                while(1) {
                    // fseek(SelectList_fp, 7, SEEK_CUR);
                    if(fscanf(SelectList_fp, "modify %s %f %s %d\n", machineItems[machineItems_index].name,
                              &machineItems[machineItems_index].price, machineItems[machineItems_index].icon_path,
                              &machineItems[machineItems_index].stock) == 4)

                    {
                        printf("商品名称：%s,商品价格：%.2f,商品图标路径：%s,商品库存：%d\n",
                               machineItems[machineItems_index].name, machineItems[machineItems_index].price,
                               machineItems[machineItems_index].icon_path, machineItems[machineItems_index].stock);
                        if(machineItems_index < 49) {
                            machineItems_index++;
                        }

                    } else {
                        printf("读取商品信息完成\n");
                        break;
                    }
                }
                // 关闭文件
                fclose(SelectList_fp);

                // 清空商品列表
                lv_obj_clean(SelectList);
                // 清空购物车列表
                lv_obj_clean(cart_list);
                // 清空购物车按钮管理结构体数组
                memset(carListBtn_manager, 0, sizeof(carListBtn_manager));
                // 清空物车按钮管理结构体下标
                carListBtn_index = 0;
                // 清空购物车
                memset(&cart, 0, sizeof(cart));
                // 添加商品到商品列表
                for(int i = 0; i < machineItems_index; i++) {
                    VendingMachineItem * item = &machineItems[i];
                    char tmp1[128]            = {0};
                    snprintf(tmp1, sizeof(tmp1), "%s\n%.2f$\nstock:%d\nAddToCar", item->name, item->price, item->stock);
                    lv_obj_t * btn = lv_list_add_btn(SelectList, item->icon_path, tmp1);
                    lv_obj_set_size(btn, 250, 120);
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0xF08080), 0);
                    lv_obj_set_style_pad_all(btn, 0, 0);
                    lv_obj_set_style_pad_top(btn, 4, 0);
                    lv_obj_set_style_border_color(btn, lv_color_hex(0xF08080), 0);
                    lv_obj_add_event_cb(btn, addGoodsButton_event_handler, LV_EVENT_ALL, item);
                }
            }
        } else if(strncmp(recvbuf1, "clear", 5) == 0) {
            // 清空商品列表
            lv_obj_clean(SelectList);
            // 清空购物车列表
            lv_obj_clean(cart_list);
            // 清空购物车按钮管理结构体数组
            memset(carListBtn_manager, 0, sizeof(carListBtn_manager));
            // 清空物车按钮管理结构体下标
            carListBtn_index = 0;
            // 清空购物车
            memset(&cart, 0, sizeof(cart));
            // 更新价格为0
            lv_label_set_text(total_label, "TotalPrice:0.00");

        } else if(strncmp(recvbuf1, "information", 12) == 0) {

            viewItemInfo();

        } else if(strncmp(recvbuf1, "advc1", 6) == 0) {
            lv_obj_del(img);
            advcpath = "S:/pan/xiaoliang/img/gif/guanggao1.gif";
            img      = lv_gif_create(playbackPanel);
            /* Assuming a File system is attached to letter 'A'
             * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
            lv_gif_set_src(img, advcpath);
            lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_width(img, 420);
            lv_obj_set_height(img, 240);

        } else if(strncmp(recvbuf1, "advc2", 6) == 0) {

            advcpath = "S:/pan/xiaoliang/img/gif/guanggao2.gif";
            img      = lv_gif_create(playbackPanel);
            /* Assuming a File system is attached to letter 'A'
             * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
            lv_gif_set_src(img, advcpath);
            lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_width(img, 420);
            lv_obj_set_height(img, 240);

        } else if(strncmp(recvbuf1, "advc3", 6) == 0) {

            advcpath = "S:/pan/xiaoliang/img/gif/guanggao3.gif";
            img      = lv_gif_create(playbackPanel);
            /* Assuming a File system is attached to letter 'A'
             * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
            lv_gif_set_src(img, advcpath);
            lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_width(img, 420);
            lv_obj_set_height(img, 240);

        } else if(strncmp(recvbuf1, "advc4", 6) == 0) {
            advcpath = "S:/pan/xiaoliang/img/gif/guanggao4.gif";
            img      = lv_gif_create(playbackPanel);
            /* Assuming a File system is attached to letter 'A'
             * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
            lv_gif_set_src(img, advcpath);
            lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_width(img, 420);
            lv_obj_set_height(img, 240);
        } else if(strncmp(recvbuf1, "exit", 5) == 0) {
            exit(0);
        }

        // 释放资源
        free(recvbuf1);
    }
}

/* 初始化所有的页面 */
void all_page_init(void)
{

    // 主页面
    strcpy(page_admin[page_num].name, "Vending_machine_home_page");
    page_admin[page_num].body_obj   = create_new_screen();
    page_admin[page_num].page_level = 0;
    page_admin[page_num].onInit     = Vending_machine_home_page_init;
    page_admin[page_num].onExit     = main_page_exit;
    cur_page                        = &page_admin[page_num];
    // 默认展示主页面，初始化主页面
    cur_page->onInit(cur_page);
    lv_scr_load(page_admin[page_num].body_obj); // 设置该页面为第一个显示在屏幕上面的页面
    page_num++;

    // 结算页面
    strcpy(page_admin[page_num].name, "settlementPage_page");
    page_admin[page_num].body_obj   = create_new_screen();
    page_admin[page_num].page_level = 1;
    page_admin[page_num].onInit     = settlementPage_init;
    page_admin[page_num].onExit     = pay_page_exit;
    page_num++;
    // 其他页面

    // 初始化购物车(置零)
    memset(&cart, 0, sizeof(cart));

    // tcp_cilent线程
    pthread_t tcp_cilent_thread;
    pthread_create(&tcp_cilent_thread, NULL, tcp_cilent_thread_func, NULL);
    pthread_detach(tcp_cilent_thread);
}
