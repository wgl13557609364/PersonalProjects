#ifndef _DIR_H
#define _DIR_H

#include "listnode.h"
P_Node NewNode( DataType * NewData );
void Add2List(P_Node Prev , P_Node New  , P_Node Next);
void DisplayList( P_Node head);
int Dir( const char * Path , const char * Type , P_Node head  );

#endif 