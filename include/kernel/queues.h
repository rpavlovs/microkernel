#ifndef __QUEUES__
#define __QUEUES__

#include "kernelspace.h"

// TODO: Description:
//       

void enqueue_task(Task_descriptor *td, Schedule *sched);

// TODO: Description:
//       

Task_descriptor *dequeue_task(Task_queue *q);

#endif
