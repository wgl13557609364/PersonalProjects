#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "listnode.h"


P_Node NewNode( DataType * NewData )
{

    P_Node New = calloc(1 , sizeof(Node));
    if ( New == NULL )
    {
        perror("calloc error");
        return NULL ;
    }

    if (NewData != NULL )
    {
        memcpy( &New->Data , NewData , sizeof (DataType) );
    }
    
    New->Next = New->Prev = New ;
    
    return New ;
}



void Add2List(P_Node Prev , P_Node New  , P_Node Next)
{
    Prev->Next = New ;
    New->Next = Next ;
    Next->Prev = New ;
    New->Prev = Prev ;

    return ;
}

void DisplayList( P_Node head)
{

    for (P_Node tmp = head->Next ; tmp != head ; tmp = tmp->Next)
    {
        printf("Path:%s Type:%c\n" , tmp->Data.FilePath , tmp->Data.Type );
    }
    
    return ;
}


int Dir( const char * Path , const char * Type , P_Node head  )
{
    int Num = 0 ;


    // 打开目录文件
    DIR * dp = opendir( Path );
    if ( dp == NULL )
    {
        fprintf( stderr , "open dir %s error:%s\n" , Path , strerror(errno));
        return 0 ;
    }

    // 检索
    while (1)
    {
        // 读取目录项 
        struct dirent * info = readdir( dp );
        if ( info == NULL )
        {
            return Num  ;
        }
        

        // 如果是普通文件则+1 
        char * p = NULL ;
        if ( info->d_type == 8 )
        {
            // 先在文件名中找到最右一个. （后缀面前面那个点）
            if (  p = strrchr(info->d_name , '.' ) )   // 
            {
                // 以已经找到的. 开始往后匹配， 看看是否符合指定类型
                if ( !strcmp(  p , Type ) )
                {
                    printf("Name:%s/%s\n" ,Path , info->d_name );

                    DataType NewData = {0} ;
                    snprintf( NewData.FilePath , 256 , "%s/%s" ,Path , info->d_name );
                    NewData.Type = *(p+1) ; // .c   .jpg  .mp3 ....


                    P_Node New = NewNode( &NewData );

                    Add2List( head->Prev , New  , head); // 尾插法

                    // Add2List( head , New  , head->Next); // 头插法

                    Num ++ ;
                }
                
            }
            
        }
        

        // 如果是目录文件则递归
        else if ( info->d_type == 4 && 
                    strcmp(info->d_name , ".") &&
                    strcmp(info->d_name , "..") )
        {
            
            char NewPath[512] = {0} ;
            snprintf( NewPath , 512 ,"%s/%s" , Path ,  info->d_name); 

            Num += Dir( NewPath , Type , head );
        }
        
    }
    
    return Num ;
}

