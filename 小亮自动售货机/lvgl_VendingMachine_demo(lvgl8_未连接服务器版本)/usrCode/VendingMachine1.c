#include "../lvgl/lvgl.h"
// #include "../lvgl/examples/lv_examples.h"
#include "dir.h"
#include <stdio.h>

// 售货机商品结构体
typedef struct
{
    char * name;
    float price;
    char * icon_path; // 商品图标地址
    int stock;        // 商品库存数量
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
    ShoppingCartItem items[11]; // 最多可放11个商品
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
VendingMachineItem machineItems[] = {
    // 图片长和宽最好是8的倍数，要不然解码能力降低，造成卡顿
    {"kele", 1.99, "S:/xiaoliang/img/commodity/1-coke.jpg", 5},
    {"weita", 2.49, "S:/xiaoliang/img/commodity/10-Vitamin_water.jpg", 3},
    {"chapi", 0.99, "S:/xiaoliang/img/commodity/11-TeaPi.jpg", 7},
    {"fenda", 3.99, "S:/xiaoliang/img/commodity/2-Fanta.jpg", 2},
    {"molicha", 3.56, "S:/xiaoliang/img/commodity/3-JasmineHoneyTea.jpg", 8},
    {"water", 4.99, "S:/xiaoliang/img/commodity/4-SpringWater.jpg", 4},
    {"binghoncha", 2.99, "S:/xiaoliang/img/commodity/5-IcedBlackTea.jpg", 6},
    {"maidong", 1.49, "S:/xiaoliang/img/commodity/7-Pulsation.jpg", 10},
    {"ADgai", 1.99, "S:/xiaoliang/img/commodity/6-ADCalciumMilk.jpg", 9},
    {"wangwang", 2.49, "S:/xiaoliang/img/commodity/8-WantWantChildren'smilk.jpg", 5}

    // 添加更多商品
};

lv_obj_t * cart_list;                               // 购物车商品列表对象
lv_obj_t * SelectList;                              // 商品选择列表对象
lv_obj_t * total_label;                             // 商品总结算金额标签对象
ShoppingCart cart;                                  // 购物车对象
lv_obj_t * msgbox;                                  // 全局变量，用于存储消息框对象
ShoppingCarBtnManager carListBtn_manager[10] = {0}; // 按钮管理数组
int carListBtn_index                         = 0;   // 管理数量（下标）

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
int Number_of_shelves = 0;         // 货架数量

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
    if(cart.count > 10) {
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
                        char buf[32];
                        snprintf(buf, sizeof(buf), "%s(%d/%d)", item->name, cart.items[i].quantity, item->stock);
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
        char buf[32];
        snprintf(buf, sizeof(buf), "%s(1/%d)", item->name, item->stock);
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

// 支付成功回调函数
static void Pay_event_cb(lv_event_t * event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_CLICKED) {
        // // 清空购物车列表
        // lv_obj_clean(cart_list);
        // 清空购物车按钮管理结构体数组
        memset(carListBtn_manager, 0, sizeof(carListBtn_manager));
        // 清空物车按钮管理结构体下标
        carListBtn_index = 0;
        // 修改商品库存数量
        for(int i = 0; i < cart.count; i++) {
            cart.items[i].item->stock -= cart.items[i].quantity;
            printf("商品名称：%s，商品库存：%d\n", cart.items[i].item->name, cart.items[i].item->stock);
        }

        // 清空购物车商品种类数
        cart.count             = 0;
        struct page * new_page = &page_admin[0];
        new_page->onInit(new_page);
        cur_page->onExit(cur_page, new_page);
        cur_page = new_page;
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
        // 清空购物车商品种类数
        cart.count = 0;
        //回到主页
        struct page * new_page = &page_admin[0];
        new_page->onInit(new_page);
        cur_page->onExit(cur_page, new_page);
        cur_page = new_page;
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
        // 清空购物车商品种类数
        cart.count = 0;
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
    lv_img_set_src(img_logo, "S:/xiaoliang/img/logo.png");
    lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 0);

    /********广告界面*******/
#if LV_BUILD_EXAMPLES
#if LV_USE_FFMPEG

    /**
     * Open a video from a file
     */

    /*birds.mp4 is downloaded from http://www.videezy.com (Free Stock Footage by Videezy!)
     *https://www.videezy.com/abstract/44864-silhouettes-of-birds-over-the-sunset*/
    lv_obj_t * player = lv_ffmpeg_player_create(machine);
    lv_ffmpeg_player_set_src(player, "/xiaoliang/videos/birds.mp4");
    lv_ffmpeg_player_set_auto_restart(player, true);
    lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_START);
    lv_obj_align(player, LV_ALIGN_BOTTOM_MID, 0, -128);
    lv_obj_set_width(player, 420);

#else

    void lv_ffmpeg_1(void)
    {
        /*TODO
         *fallback for online examples*/

        lv_obj_t * label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "FFmpeg is not installed");
        lv_obj_center(label);
    }

#endif
#endif

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
    // /*抗锯齿*/
    // lv_img_set_antialias(SelectList, true);
    // 添加售货机商品列表项
    for(int i = 0; i < sizeof(machineItems) / sizeof(machineItems[0]); i++) {
        VendingMachineItem * item = &machineItems[i];
        char buf[32];
        snprintf(buf, sizeof(buf), "%s\n%.2f$\nstock:%d\nAddToCar", item->name, item->price,item->stock);
        lv_obj_t * btn = lv_list_add_btn(SelectList, item->icon_path, buf);
        lv_obj_set_size(btn, 250, 120);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xF08080), 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_set_style_pad_top(btn, 4, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xF08080), 0);
        lv_obj_add_event_cb(btn, addGoodsButton_event_handler, LV_EVENT_ALL, item);
    }
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
    lv_img_set_src(ShoppingCart, "S:/xiaoliang/img/CartPgColor.png");
    lv_obj_align(ShoppingCart, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(ShoppingCart, lv_color_hex(0xFF7F50), 0);//背景颜色
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
    lv_img_set_src(img_weixin, "S:/xiaoliang/img/CollectionCode/weixinshoukuanma.png");
    lv_obj_align(img_weixin, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(weixinQrcodeBtn, Pay_event_cb, LV_EVENT_ALL, NULL);

//支付宝支付
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
    lv_img_set_src(img_zhifubao, "S:/xiaoliang/img/CollectionCode/zhifubaoshoukma.jpg");
    lv_obj_align(img_zhifubao, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(zhifubaoQrcodeBtn, Pay_event_cb, LV_EVENT_ALL, NULL);

    //取消支付按钮
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
}
