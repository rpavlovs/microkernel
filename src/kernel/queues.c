#include "kernelspace.h"

void enqueue_tqueue(Task_descriptor *td, Task_queue *q){
	assert(q->size != MAX_NUM_TASKS, "Task queue should not overflow :)");

	//Modifying the queue
	q->size++;
	if(++(q->newest) == MAX_NUM_TASKS) q->newest = 0;
	q->td_ptrs[q->newest] = td;
}

Task_descriptor *dequeue_tqueue(Task_queue *q){
	assert(q->size != 0, "Task queue should have items to dequeue");

	//Modifying the queue
	q->size--;
	Task_descriptor *td = q->td_ptrs[q->oldest];
	if(++(q->oldest) == MAX_NUM_TASKS) q->oldest = 0;
	return td;
}