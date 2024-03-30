#ifndef _LIST_H_
#define _LIST_H_

#include "datatype.h"

pNode getNewNode(dataType *data);
void addlist(pNode Prev, pNode newNode ,pNode Next);
void printlist(pNode head);
void destroyList(pNode head);

#endif