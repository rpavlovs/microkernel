#ifndef __QUEUES__
#define __QUEUES__

#include "kernelspace.h"

// TODO: Description:
//       

void enqueue_tqueue(Task_descriptor *td, Task_queue *q);

// TODO: Description:
//       

Task_descriptor *dequeue_tqueue(Task_queue *q);

#endif
