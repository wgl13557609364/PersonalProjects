#ifndef _DATATYPE_H_
#define _DATATYPE_H_

typedef struct dataType
{
    char name[100];
    char type[20];
}dataType;

typedef struct node
{
    dataType data;
    struct node *next,*prev;
}Node,*pNode;

extern int fd_lcd;
extern int * map;



#endif