#ifndef     __MY_TYPES_H_
#define     __MY_TYPES_H_


// 链表数据域的设计
typedef  struct fileInfo
{
    char FilePath [256] ;
    char Type ; // 'c' C源文件  'j' Jpg 图像文件  'm' 音乐文件 ......
}DataType ;

// 链表结点的设计
typedef struct Node
{
    DataType Data ;

    struct Node * Prev , * Next ;
}Node , *P_Node ;



#endif