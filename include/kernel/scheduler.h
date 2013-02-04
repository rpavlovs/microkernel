#ifndef ___SCHED___
#define ___SCHED___

#include "kernelspace.h"

// Description:
// Represents a node in main priority queue
typedef struct Node
{
    Task_descriptor *td;
    struct Node *next;
    struct Node *previous;
} Node;

// Description
// Represents main priority queue
typedef struct
{
    Node nodes[102];
    Node *deadend;
    Node *sentinel;
    Node *last_in_priority[16];
} Priority_queue;

#endif
