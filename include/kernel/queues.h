#ifndef __QUEUES__
#define __QUEUES__

#include "kernelspace.h"

void enqueue_tqueue(Task_descriptor *td, Task_queue *q);

Task_descriptor *dequeue_tqueue(Task_queue *q);

#endif
