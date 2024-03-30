#include <stdlib.h>
#include "datatype.h"
#include <string.h>
#include <stdio.h>
pNode getNewNode(dataType *data)
{
    pNode newNode = calloc(1, sizeof(Node));
    if (newNode == NULL)
    {
        perror("申请内存失败！");
        return NULL;
    }
    if (data != NULL)
    {
        memcpy(&newNode->data, data, sizeof(dataType));
    }

    newNode->next = newNode->prev = newNode;

    return newNode;
}

void addlist(pNode Prev, pNode newNode, pNode Next)
{
    Prev->next = newNode;
    newNode->next = Next;
    Next->prev = newNode;
    newNode->prev = Prev;
}

void printlist(pNode head)
{
    pNode tmp = head->next;
    while (tmp != head)
    {
        printf("%s[%s]\n", tmp->data.name, tmp->data.type);
        tmp = tmp->next;
    }
}

// 销毁链表
void destroyList(pNode head)
{
    if (head == NULL)
        return;

    pNode current = head->next;
    while (current != head)
    {
        pNode nextNode = current->next;
        free(current); // 释放节点的内存
        current = nextNode;
    }

    free(head); // 释放头节点的内存
}
