#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>

int cilent_fd = -1;

/// @brief :菜单
void Menu()
{
    printf("******* please choose what you do   *******\n");
    printf("******* 1.给售货机上架商品            ******\n");
    printf("******* 2.查看订单信息                ******\n");
    printf("******* 3.查看商品信息                ******\n");
    printf("******* 4.修改广告                    ******\n");
    printf("******* 5.清空售货机                  ******\n");
    printf("******* 6.停用售货机                  ******\n");
    printf("\n");
}

/// @brief :上架商品
void ModifyMerchandise()
{
    FILE *fp = fopen("ModifyCommodity.txt", "r");
    if (NULL == fp)
    {
        perror("fp Modify open error");
        return;
    }
    // 获取源文件的大小
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 分配足够的内存来存储整个文件内容
    char *buffer = calloc(fileSize + 1, sizeof(char));
    if (buffer == NULL)
    {
        perror("内存分配失败");
        fclose(fp);
        return;
    }

    // 一次性读取整个文件内容到缓冲区中
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, fp);
    if (bytesRead != fileSize)
    {
        perror("读取文件失败");
        free(buffer);
        fclose(fp);
        return;
    }

    // 发送给售货机
    buffer[fileSize] = '\0'; // 添加字符串结束符
    if (write(cilent_fd, buffer, fileSize) > 0)
    {
        // printf("发送成功\n");
    }
    else
    {
        // printf("发送失败\n");
    }
    // 清理资源
    free(buffer);
    fclose(fp);
    return;
}

//清空售货机
void clearMerchandise()
{

    if (write(cilent_fd, "clear", 6) > 0)
    {
        // printf("发送成功\n");
    }
}

//查看订单函数
void GetOrderInformation()
{
    //打开订单文件
    FILE *Order_fp= fopen("Order.txt", "r");
    if (NULL == Order_fp)
    {
        printf("还没有订单生成！\n");
        return;
    }
    fseek(Order_fp, 0, SEEK_END);
    int Order_len = ftell(Order_fp);
    fseek(Order_fp, 0, SEEK_SET);
    char *Order_buffer = calloc(Order_len + 1, sizeof(char));
    if (NULL == Order_buffer)
    {
        perror("Order_buffer calloc error");
    }
    fread(Order_buffer, sizeof(char), Order_len, Order_fp);
    printf("%s", Order_buffer);
    free(Order_buffer);
    //关闭订单文件
    fclose(Order_fp);
    return;
}

//查看商品信息
void MerchandiseInformation()
{
    if (write(cilent_fd, "information", 12) > 0)
    {
        // printf("发送成功\n");
    }

}

//修改广告
void ModifyAdvertisement()
{
    printf("这里是广告修改，请选择广告.......\n");
    printf("1.可口可乐\n");
    printf("2.猫和老鼠\n");
    printf("3.IKUN\n");
    printf("4.老配比\n");
    int i = 0;
    scanf("%d", &i);

    char buffer[10] = {0};
    switch (i)
    {
    case 1:
        printf("你选择了可口可乐\n");
        memcpy(buffer, "advc1", 6);
        write(cilent_fd, buffer, strlen(buffer));
        break;
    case 2:
        printf("你选择了猫和老鼠\n");
        memcpy(buffer, "advc2", 6);
        write(cilent_fd, buffer, strlen(buffer));
        break;
    case 3:
        printf("你选择了IKUN\n");
        memcpy(buffer, "advc3", 6);
        write(cilent_fd, buffer, strlen(buffer));
        break;
    case 4:
        printf("你选择了老配比\n");
        memcpy(buffer, "advc4", 6);
        write(cilent_fd, buffer, strlen(buffer));
        break;
    default:
        printf("你没有选择广告\n");
        break;
    }
    printf("\n");
}

void exit_fun()
{
    if (write(cilent_fd, "exit", 5) > 0)
    {
        // printf("发送成功\n");
    }
}




/// @brief     :接收信息线程
/// @param arg :
void *RecvData(void *arg)
{
    int sockfd = *(int *)arg;

    char recvbuf[1024*1024] = {0};
    while (1)
    {
        bzero(recvbuf, 1024*1024);
        if (read(sockfd, recvbuf, sizeof(recvbuf)) > 0)
        {
            // printf("%s\n", recvbuf);
            if (NULL != strstr(recvbuf, "订单编号"))
            {
                FILE * Order_fp = fopen("Order.txt", "a+");
                if (Order_fp == NULL)
                {
                    perror("Order.txt open failed");
                }
                int order_val = fwrite(recvbuf, sizeof(char), strlen(recvbuf), Order_fp);
                if (order_val > 0)
                {
                    // printf("写入成功\n");
                }
                else
                {
                    // printf("写入失败\n");
                }
                fclose(Order_fp);
            }
            if (NULL != strstr(recvbuf, "商品详情"))
            {
                printf("%s", recvbuf);
                break;
            }
            
        }
    }
}

int main()
{
    // 1.创建TCP 服务器
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 使套接字sockfd关联的地址在套接字关闭后立即释放
    int on = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("设置地址服用失败\n");
        return -1;
    }

    // 2.绑定服务器地址信息
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(tcp_socket, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("绑定服务器失败\n");
        return -1;
    }

    // 3.设置为监听模式
    int listen_val = listen(tcp_socket, 10);
    if (listen_val < 0)
    {
        perror("监听失败\n");
        return -1;
    }

    // 4.接受客户端连接
    cilent_fd = accept(tcp_socket, NULL, NULL);
    if (cilent_fd < 0)
    {
        perror("处理连接请求失败\n");
        return -1;
    }
    else
    {
        printf("客户端已连接\n");
    }

    // 创建一个线程用来接收数据
    pthread_t tidr;
    pthread_create(&tidr, NULL, RecvData, (void *)&cilent_fd);

    while (1)
    {

        while (1)
        {

            Menu();
            int i;
            scanf("%d", &i);

            switch (i)
            {
            case 1:
                ModifyMerchandise();
                break;
            case 2:
                GetOrderInformation();
                break;
            case 3:
                MerchandiseInformation();
                break;
            case 4:
                ModifyAdvertisement();
                break;
            case 5:
                clearMerchandise();
                break;
            case 6:
                exit_fun();
                break;
            default:
                printf("please enter the correct option...\n");
                break;
            }
        }
    }
    // 关闭通信
    close(tcp_socket);
    return 0;
}