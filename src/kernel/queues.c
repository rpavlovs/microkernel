#include "kernelspace.h"

void enqueue_task(Task_descriptor *td, Task_queue *q){
	// ASSERT: Verifying the size of the queue
	assert( q->size < SCHED_QUEUE_LENGTH, "Task queue should not overflow :)" );

	//Checking for empty queue
	if (q->size == 0)
	{
		q->newest = 0;
		q->oldest = 0;
	}
	//Checking for queue boundaries
	else if (++(q->newest) >= SCHED_QUEUE_LENGTH)
	{
		q->newest = 0;
	}

	// Updating the queue
	q->size++;
	q->td_ptrs[q->newest] = td;
}

Task_descriptor *dequeue_tqueue(Task_queue *q){
	// ASSERT: Verifying the size of the queue
	assert( q->size > 0, "Task queue should have items to dequeue" );

	//Modifying the queue
	q->size--;
	Task_descriptor *td = q->td_ptrs[q->oldest];
	if(++(q->oldest) == MAX_NUM_TASKS) q->oldest = 0;
	return td;
}
