// #include "../lvgl/lvgl.h"
// #include "dir.h"
// #include <stdio.h>

// // 售货机商品结构体
// typedef struct
// {
//     char * name;      // 商品名字
//     float price;      // 商品价格
//     char * icon_path; // 商品图标地址
//     int stock;        // 商品库存数量
// } VendingMachineItem;

// // 购物车商品结构体
// typedef struct
// {
//     VendingMachineItem * item; // 商品种类
//     int quantity;              // 商品种类数目
// } ShoppingCartItem;

// // 购物车结构体
// typedef struct
// {
//     ShoppingCartItem items[11]; // 最多可放11个商品
//     int count;                  // 购物车商品种类数量
// } ShoppingCart;

// // 按钮管理结构体
// typedef struct
// {
//     lv_obj_t * btn;
//     VendingMachineItem * item;
//     lv_obj_t * btn_label;
// } ShoppingCarBtnManager;

// // 售货机商品列表
// VendingMachineItem machineItems[] = {
//     // 图片长和宽最好是8的倍数，要不然解码能力降低，造成卡顿
//     {"Coke", 1.99, "S:/mnt/pan/img/commodity/1-coke.jpg", 5},
//     {"VitaminWater", 2.49, "S:/mnt/pan/img/commodity/10-Vitamin_water.jpg", 3},
//     {"TeaPi", 0.99, "S:/mnt/pan/img/commodity/11-TeaPi.jpg", 7},
//     {"Fanta", 3.99, "S:/mnt/pan/img/commodity/2-Fanta.jpg", 2},
//     {"JasmineHoneyTea", 3.56, "S:/mnt/pan/img/commodity/3-JasmineHoneyTea.jpg", 8},
//     {"SpringWater", 4.99, "S:/mnt/pan/img/commodity/4-SpringWater.jpg", 4},
//     {"IcedBlackTea", 2.99, "S:/mnt/pan/img/commodity/5-IcedBlackTea.jpg", 6},
//     {"Pulsation", 1.49, "S:/mnt/pan/img/commodity/7-Pulsation.jpg", 10},
//     {"ADCalciumMilk", 1.99, "S:/mnt/pan/img/commodity/6-ADCalciumMilk.jpg", 9},
//     {"WantWantChildren'smilk", 2.49, "S:/mnt/pan/img/commodity/8-WantWantChildren'smilk.jpg", 5}

//     // 添加更多商品
// };

// // 定义一个结构体，用来存货架号和对应的商品图片路径
// typedef struct NumberAndPicture
// {
//     uint32_t id;
//     char picture_path[512];
//     uint32_t price;
// } NumberAndPicture;

// // 相关全局变量
// lv_obj_t * Shopping_Cart;                             // 购物车商品列表对象
// lv_obj_t * total_label;                               // 商品总结算金额标签对象
// ShoppingCart cart;                                    // 购物车结构体对象
// lv_obj_t * msgbox;                                    // 全局变量，用于存储消息框对象
// ShoppingCarBtnManager Shopping_btn_manager[11] = {0}; // 购物车按钮管理数组
// int Shopping_btn_manager_count                 = 0;   // 购物车按钮管理数量

// // 页面管理
// /**
//  *
//  * 页面管理，前期一定要准备好。我们在实际使用 LVGL 完成一些项目时
//  * ，通常需要展示不止一个页面，此时这些页面要如何更好的进行管理成为
//  * 了一个需要解决的问题，如果处理不当，在资源短缺的嵌入式设备中很可
//  * 能会因为过多页面的加载但却没有及时释放造成系统的崩溃。以下是我准备
//  * 的页面管理框架。
//  *
//  * 前期了解：
//  *  屏幕（页面）是没有父对象的特殊对象。所以它们可以像这样创建
//  * lv_obj_t *page = lv_obj_create(NULL);
//  * 加载想要显示的屏幕（页面）
//  * lv_scr_load(page);
//  * 删除对象的所有子项（但不是对象本身）
//  * lv_obj_clean(page);
//  *  动画加载屏幕（页面）
//  * lv_scr_load_anim(page, transition_type, time, delay, false);
//  * transition_type：LV_SCR_LOAD_ANIM_NONE：在 delay 毫秒后立即切换
//  *                  LV_SCR_LOAD_ANIM_OVER_LEFT/RIGHT/TOP/BOTTOM：将新屏幕移动到当前的指定方向
//  *                  LV_SCR_LOAD_ANIM_MOVE_LEFT/RIGHT/TOP/BOTTOM：将当前屏幕和新屏幕都向给定方向移动
//  *                  LV_SCR_LOAD_ANIM_FADE_ON：在旧屏幕上淡出新屏幕
//  *
//  * */

// // 页面管理结构体
// struct page
// {
//     char name[50];       // 页面名
//     int page_level;      // 页面等级，依靠这个来实现对上一页面的切换，该值越小，等级越高
//     lv_obj_t * body_obj; // 页面结构体
//     void * private_data; // 私有数据
//     void (*onInit)(struct page * page);                             // 创建页面函数
//     void (*onExit)(struct page * old_page, struct page * new_page); // 销毁页面函数
// };

// // 页面管理其他相关变量
// static struct page page_admin[50]; // 存储所有创建好的页面
// static int page_num = 0;           // 记录 page_admin[] 数组中存在多少个页面
// struct page * cur_page;            // 记录当前显示的页面
// int Number_of_shelves = 0;         // 货架数量

// // 查找页面
// static struct page * page_search(char * name)
// {
//     for(int i = 0; i < page_num; i++) {
//         if(strcmp(page_admin[i].name, name) == 0) return &page_admin[i];
//     }
//     return NULL;
// }

// /**
//  * @brief 页面切换函数
//  * @param new_page_name 新页面名
//  * @return 0 成功     -1 新页面不存在
//  */
// int page_transition(char * new_page_name)
// {
//     printf("transition page = %s\n", new_page_name);
//     if(strcmp(new_page_name, "") == 0 || new_page_name == NULL) return -1;

//     struct page * new_page = page_search(new_page_name);
//     if(new_page != NULL) {
//         cur_page->onExit(cur_page, new_page);
//         new_page->onInit(new_page);
//         cur_page = new_page;
//         return 0;
//     } else {
//         return -1;
//     }
// }

// /**
//  * @brief 返回上一页面
//  * @param page 当前页面
//  * @return struct page 指针 上一页面    NULL 失败
//  */
// struct page * return_prev_page(struct page * page)
// {
//     if(page == NULL) return NULL;

//     int cur_page_level = page->page_level;
//     for(int i = page_num - 1; i >= 0; i--) {
//         if(page_admin[i].page_level < cur_page_level) {
//             return &page_admin[i];
//         }
//     }
//     return NULL;
// }

// /* 设计页面的样式 */
// // void style_init(void) {
// //     lv_style_init(&page_style);
// //     lv_style_set_bg_color(&page_style, lv_color_hex(0x000000));
// //     lv_style_set_radius(&page_style, 0);
// //     lv_style_set_pad_all(&page_style, 0);
// //     lv_style_set_border_side(&page_style, LV_BORDER_SIDE_NONE);
// // }

// /* 创建一个指定样式的空白屏幕 */
// lv_obj_t * create_new_screen(void)
// {
//     lv_obj_t * main_obj = lv_obj_create(NULL);
//     // lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
//     lv_obj_clean(main_obj);
//     lv_obj_set_size(main_obj, 800, 480); // 根据个人屏幕大小修改
//     // lv_obj_add_style(main_obj, &page_style, 0);

//     return main_obj;
// }

// // 添加商品到购物车
// void add_item_to_cart(VendingMachineItem * item)
// {
//     if(cart.count > 10) {
//         // 购物车已满，无法添加更多商品
//         printf("购物车已满,无法添加更多商品！\n");
//         return;
//     }

//     // 检查购物车中是否已有相同商品
//     for(int i = 0; i < cart.count; i++) {
//         if(cart.items[i].item == item) {
//             // 商品已存在购物车中
//             if(cart.items[i].quantity < item->stock) {
//                 // 增加数量
//                 cart.items[i].quantity++;
//                 // 查找管理结构体
//                 for(int j = 0; j < Shopping_btn_manager_count; j++) {
//                     if(Shopping_btn_manager[j].item == item) {

//                         // 更新购物车列表显示
//                         char buf[32]={0};
//                         snprintf(buf, sizeof(buf), "%s(%d/%d)", item->name, cart.items[i].quantity, item->stock);
//                         lv_label_set_text(Shopping_btn_manager[j].btn_label, buf);
//                         // lv_label_set_text_fmt(Shopping_btn_manager[j].btn_label, "%s", buf);
//                         // lv_obj_clean(Shopping_btn_manager[j].btn);
//                         // lv_obj_del(Shopping_btn_manager[j].btn);
//                         // update_total_label();
//                         break;
//                     }
//                 }

//                 // lv_obj_del;
//                 // lv_obj_get_index;
//                 // lv_obj_move_to_index;

//                 // 减少库存量
//             }
//             // update_total_label(); // 更新商品总结算金额
//             return;
//         }
//     }

//     // 商品未在购物车中，则添加新的购物车商品
//     if(item->stock > 0) {
//         ShoppingCartItem * cartItem = &cart.items[cart.count];
//         cartItem->item              = item;
//         cartItem->quantity          = 1;
//         cart.count++;

//         // 更新购物车列表显示
//         static lv_style_t style3;
//         lv_style_init(&style3);
//         // 设置标签字体颜色
//         lv_style_set_text_color(&style3, lv_color_hex(0x000000));

//         lv_obj_t * btn;
//         lv_obj_t * label;

//         /*Add items to the row*/
//         char buf[32] = {0};
//         snprintf(buf, sizeof(buf), "%s(1/%d)", item->name, item->stock);
//         btn   = lv_btn_create(Shopping_Cart);
//         label = lv_label_create(Shopping_Cart);
//         lv_label_set_text(label, item->name);

//         printf("item->name:%s\n", item->name);
        
        
//         /* 取消按钮所有的样式属性 */
//         lv_obj_remove_style_all(btn);
//         // 设置按下状态的透明度为默认状态的透明度 /* 设置控件透明度 */
//         lv_obj_set_style_bg_opa(btn, 255, LV_STATE_PRESSED);
//         lv_obj_set_style_bg_opa(btn, 255, LV_STATE_DEFAULT);                      // 默认
//         lv_obj_set_style_bg_color(btn, lv_color_hex(0x000000), LV_STATE_DEFAULT); // 设置按钮背景
//         // 设置按钮弧度
//         lv_obj_set_style_radius(btn, 15, LV_STATE_DEFAULT);
//         lv_obj_set_style_radius(btn, 15, LV_STATE_PRESSED);

//         // 设置按下状态下的阴影样式为透明
//         //  lv_obj_set_style_shadow_opa(obj, 0, LV_STATE_PRESSED);
//         //  设置按下状态下的阴影颜色为透明 lv_obj_set_style_shadow_color();
//         //  设置按下状态下的阴影颜色为透明 lv_obj_set_style_shadow_opa(&style_pressed, LV_OPA_TRANSP, LV_STATE_PRESSED);

//         // void lv_img_set_antialias(lv_obj_t * obj, bool antialias) //抗锯齿函数
//        //应用样式
//         lv_obj_add_style(btn, &style3, 0);
//          // 设置按钮大小
//         lv_obj_set_width(btn, 210);
//         lv_obj_set_height(btn, 290);
//         // lv_obj_set_size(obj, 210, LV_PCT(75));
//         lv_label_set_text_fmt(label, "%s", buf);
//         lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 15);

//         Shopping_btn_manager[Shopping_btn_manager_count].btn       = btn;
//         Shopping_btn_manager[Shopping_btn_manager_count].item      = item;
//         Shopping_btn_manager[Shopping_btn_manager_count].btn_label = label;
//         Shopping_btn_manager_count++;

//         // update_total_label(); // 更新商品总结算金额
//     }
// }

// // 添加按钮回调函数
// void btn_event_handler(lv_event_t * event)
// {
//     lv_event_code_t code      = lv_event_get_code(event);
//     VendingMachineItem * item = lv_event_get_user_data(event);
//     if(code == LV_EVENT_CLICKED) {
//         printf("商品按钮被点击\n");
//         // 在这里处理按钮点击事件的逻辑
//         add_item_to_cart(item);
//     }
// }

// // 购物车按钮事件
// static void cart_btn_event_handler(lv_event_t * event)
// {
//     lv_event_code_t code = lv_event_get_code(event);
//     if(code == LV_EVENT_CLICKED) {
//         // printf("购物车按钮被点击\n");
//         // 在这里处理购物车按钮点击事件的逻辑
//         struct page * new_page = &page_admin[1];
//         cur_page->onExit(cur_page, new_page);
//         new_page->onInit(new_page);
//         cur_page = new_page;
//     }
// }

// #if LV_USE_FLEX && LV_BUILD_EXAMPLES

// // 商品主页面
// void merchandise_init(struct page * page)
// {
//     // 给弹性流容器添加属性
//     static lv_style_t style1;
//     lv_style_init(&style1);
//     lv_style_set_pad_all(&style1, 0); // 填充(Padding)可在边缘的内侧设置空间
//     lv_style_set_radius(&style1, 0);  // 四角弧度
//     // lv_style_set_border_opat(&style1, LV_OPA_COVER);
//     // lv_style_set_border_width(&style1, 0);//边框宽度
//     // lv_style_set_border_color(&style1, lv_color_hex(0xF0FFF0));//边框颜色
//     // lv_style_set_border_side(&style1, LV_BORDER_SIDE_NONE);//指定要绘制边框的哪一测
//     lv_style_set_border_opa(&style1, 0x0);                  // 边框透明度 LV_OPA_COVER
//     lv_style_set_bg_color(&style1, lv_color_hex(0x836FFF)); // 背景
//     // lv_style_set_bg_opa(&style1, 0x0);//背景透明度 LV_OPA_COVER
//     // lv_style_set_text_color(&style1, LV_COLOR_BLACK);
//     // lv_style_set_text_font(&style1, &lv_font_montserrat_16);

//     /*Create a container with ROW flex direction*/

//     lv_obj_t * body_obj = page->body_obj;
//     lv_obj_t * cont_row = lv_obj_create(body_obj);

//     LV_IMG_DECLARE(img_wink_png);
//     lv_obj_t * img_logo;
//     img_logo = lv_img_create(body_obj);
//     lv_img_set_src(img_logo, &img_wink_png);
//     lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 0);

//     img_logo = lv_img_create(body_obj);
//     /* Assuming a File system is attached to letter 'A'
//      * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
//     lv_img_set_src(img_logo, "S:/mnt/pan/img/logo.png");
//     lv_obj_align(img_logo, LV_ALIGN_TOP_LEFT, 0, 0);

//     lv_obj_t * cartbtn = lv_btn_create(body_obj);
//     lv_obj_set_size(cartbtn, 150, 150);
//     lv_obj_t* carlable = lv_label_create(cartbtn);
//     lv_label_set_text(carlable, "ShoppingCar");
//     lv_obj_center(carlable);
//     lv_obj_align(cartbtn, LV_ALIGN_TOP_RIGHT, 0, 0);
//     lv_obj_add_event_cb(cartbtn, cart_btn_event_handler, LV_EVENT_ALL, NULL);

//     // 添加属性并设置属性优先级
//     lv_obj_add_style(cont_row, &style1, 0);

//     lv_obj_set_size(cont_row, 800, 330);
//     lv_obj_align(cont_row, LV_ALIGN_BOTTOM_MID, 0, 0);
//     lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
//     // 将弹性流容器中的对象在垂直方向上都居中对齐
//     // 第一参数：要设置对齐方式的弹性流容器对象 第二参数：默认对齐方向 第三参数：水平对齐方式 第四参数：垂直对齐方式
//     lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

//     // //最多能添加的货架
//     // if (Number_of_shelves < 10)
//     // {
//     //     /* code */
//     // }

//     // 添加商品
//     for(int i = 0; i < sizeof(machineItems) / sizeof(machineItems[0]); i++) {
//         VendingMachineItem * item = &machineItems[i];
//         char buf[32]={0};
//         snprintf(buf, sizeof(buf), "%s\n%.2f", item->name, item->price);

//         static lv_style_t style2;
//         lv_style_init(&style2);
//         // 设置标签字体颜色
//         lv_style_set_text_color(&style2, lv_color_hex(0x000000));

//         lv_obj_t * btn;
//         lv_obj_t * label1;

//         /*Add items to the row*/
//         btn = lv_btn_create(cont_row);
//         // 设置按钮事件
//         lv_obj_add_event_cb(btn, btn_event_handler, LV_EVENT_ALL, item);
//         lv_obj_add_style(btn, &style2, 0);
//         /* 取消按钮所有的样式属性 */
//         lv_obj_remove_style_all(btn);
//         // 设置按下状态的透明度为默认状态的透明度 /* 设置控件透明度 */
//         lv_obj_set_style_bg_opa(btn, 255, LV_STATE_PRESSED);
//         lv_obj_set_style_bg_opa(btn, 255, LV_STATE_DEFAULT);                      // 默认
//         lv_obj_set_style_bg_color(btn, lv_color_hex(0xF08080), LV_STATE_DEFAULT); // 设置按钮背景
//         // 设置按钮弧度
//         lv_obj_set_style_radius(btn, 15, LV_STATE_DEFAULT);
//         lv_obj_set_style_radius(btn, 15, LV_STATE_PRESSED);

//         // 设置按下状态下的阴影样式为透明
//         //  lv_obj_set_style_shadow_opa(obj, 0, LV_STATE_PRESSED);
//         //  设置按下状态下的阴影颜色为透明 lv_obj_set_style_shadow_color();
//         //  设置按下状态下的阴影颜色为透明 lv_obj_set_style_shadow_opa(&style_pressed, LV_OPA_TRANSP, LV_STATE_PRESSED);

//         // void lv_img_set_antialias(lv_obj_t * obj, bool antialias) //抗锯齿函数
//         // 设置按钮大小
//         lv_obj_set_width(btn, 210);
//         lv_obj_set_height(btn, 290);
//         // lv_obj_set_size(obj, 210, LV_PCT(75));
//         label1 = lv_label_create(btn);
//         lv_label_set_text_fmt(label1, "%s", buf);
//         lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 15);

//         LV_IMG_DECLARE(img_wink_png);
//         lv_obj_t * img1;
//         img1 = lv_img_create(btn);
//         lv_img_set_src(img1, &img_wink_png);
//         lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
//         img1 = lv_img_create(btn);
//         /* Assuming a File system is attached to letter 'A'
//          * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */

//         lv_img_set_src(img1, item->icon_path);
//         lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
//     }
// }

// // 购物车界面
// void Shopping_Cart_init(struct page * page)
// {
//     // 给弹性流容器添加属性
//     static lv_style_t style1;
//     lv_style_init(&style1);
//     lv_style_set_pad_all(&style1, 0); // 填充(Padding)可在边缘的内侧设置空间
//     lv_style_set_radius(&style1, 0);  // 四角弧度
//     // lv_style_set_border_opat(&style1, LV_OPA_COVER);
//     // lv_style_set_border_width(&style1, 0);//边框宽度
//     // lv_style_set_border_color(&style1, lv_color_hex(0xF0FFF0));//边框颜色
//     // lv_style_set_border_side(&style1, LV_BORDER_SIDE_NONE);//指定要绘制边框的哪一测
//     lv_style_set_border_opa(&style1, 0x0);                  // 边框透明度 LV_OPA_COVER
//     lv_style_set_bg_color(&style1, lv_color_hex(0x836FFF)); // 背景
//     // lv_style_set_bg_opa(&style1, 0x0);//背景透明度 LV_OPA_COVER
//     // lv_style_set_text_color(&style1, LV_COLOR_BLACK);
//     // lv_style_set_text_font(&style1, &lv_font_montserrat_16);

//     /*Create a container with ROW flex direction*/

//     lv_obj_t * body_obj = page->body_obj;
//     Shopping_Cart       = lv_obj_create(body_obj);
//     LV_IMG_DECLARE(img_wink_png);
//     lv_obj_t * img_logo;
//     img_logo = lv_img_create(body_obj);
//     lv_img_set_src(img_logo, &img_wink_png);
//     lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 0);

//     img_logo = lv_img_create(body_obj);
//     /* Assuming a File system is attached to letter 'A'
//      * E.g. set LV_USE_FS_STDIO 'A' in lv_conf.h */
//     lv_img_set_src(img_logo, "S:/mnt/pan/img/logo.png");
//     lv_obj_align(img_logo, LV_ALIGN_TOP_MID, 0, 0);

//     // 添加属性并设置属性优先级
//     lv_obj_add_style(Shopping_Cart, &style1, 0);

//     lv_obj_set_size(Shopping_Cart, 800, 330);
//     lv_obj_align(Shopping_Cart, LV_ALIGN_BOTTOM_MID, 0, 0);
//     lv_obj_set_flex_flow(Shopping_Cart, LV_FLEX_FLOW_ROW);
//     // 将弹性流容器中的对象在垂直方向上都居中对齐
//     // 第一参数：要设置对齐方式的弹性流容器对象 第二参数：默认对齐方向 第三参数：水平对齐方式 第四参数：垂直对齐方式
//     lv_obj_set_flex_align(Shopping_Cart, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
// }

// void page_exit(struct page * old_page, struct page * new_page)
// {
//     lv_obj_clean(old_page->body_obj);
//     lv_scr_load_anim(new_page->body_obj, LV_SCR_LOAD_ANIM_MOVE_TOP, 500, 0, false);
// }

// #endif

// /* 初始化所有的页面 */
// void all_page_init(void)
// {
//     // style_init();

//     // 主页面
//     strcpy(page_admin[page_num].name, "merchandise_page");
//     page_admin[page_num].body_obj   = create_new_screen();
//     page_admin[page_num].page_level = 0;
//     page_admin[page_num].onInit     = merchandise_init;
//     page_admin[page_num].onExit     = page_exit;
//     cur_page                        = &page_admin[page_num];
//     // 默认展示主页面
//     cur_page->onInit(cur_page);
//     lv_scr_load(page_admin[page_num].body_obj); // 设置该页面为第一个显示在屏幕上面的页面
//     page_num++;

//     // 购物车页面
//     strcpy(page_admin[page_num].name, "merchandise_page");
//     page_admin[page_num].body_obj   = create_new_screen();
//     page_admin[page_num].page_level = 1;
//     page_admin[page_num].onInit     = Shopping_Cart_init;
//     page_admin[page_num].onExit     = page_exit;
//     page_num++;
//     // 结算页面
// }

// #if LV_USE_QRCODE && LV_BUILD_EXAMPLES

// /**
//  * Create a QR Code
//  */
// void lv_example_qrcode_test1(void)
// {
//     lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 5);
//     lv_color_t fg_color = lv_palette_darken(LV_PALETTE_YELLOW, 4);

//     lv_obj_t * qr = lv_qrcode_create(lv_scr_act(), 150, fg_color, bg_color);

//     /*Set data*/
//     const char * data = "http://vm.yueqian.com.cn:8886/index.html";
//     lv_qrcode_update(qr, data, strlen(data));
//     lv_obj_center(qr);

//     /*Add a border with bg_color*/
//     lv_obj_set_style_border_color(qr, bg_color, 0);
//     lv_obj_set_style_border_width(qr, 5, 0);
// }

// #endif

// #if LV_BUILD_EXAMPLES
// #if LV_USE_FFMPEG

// /**
//  * Open a video from a file
//  */
// void lv_ffmpeg_1(void)
// {
//     /*birds.mp4 is downloaded from http://www.videezy.com (Free Stock Footage by Videezy!)
//      *https://www.videezy.com/abstract/44864-silhouettes-of-birds-over-the-sunset*/
//     lv_obj_t * player = lv_ffmpeg_player_create(lv_scr_act());
//     lv_ffmpeg_player_set_src(player, "/mnt/pan/videos/birds.mp4");
//     lv_ffmpeg_player_set_auto_restart(player, true);
//     lv_ffmpeg_player_set_cmd(player, LV_FFMPEG_PLAYER_CMD_START);
//     lv_obj_center(player);
// }

// #else

// void lv_ffmpeg_1(void)
// {
//     /*TODO
//      *fallback for online examples*/

//     lv_obj_t * label = lv_label_create(lv_scr_act());
//     lv_label_set_text(label, "FFmpeg is not installed");
//     lv_obj_center(label);
// }

// #endif
// #endif
