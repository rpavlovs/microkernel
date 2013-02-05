#include "kernelspace.h"

void enqueue_tqueue(Task_descriptor *td, Task_queue *q){
	// ASSERT: Verifying the size of the queue
	assert( q->size < SCHED_QUEUE_LENGTH, "Scheduler queue must not be full" );

	// If the queue is empty or the newest pointer is at the end of the td_ptrs buffer
	// put the next td_ptr at the beginning on the buffer  
	if (q->size == 0 || ++(q->newest) >= SCHED_QUEUE_LENGTH) q->newest = 0;
	
	// If the queue was empty then newest and oldest elements are the same 
	// and are at the beginning of the buffer
	if (q->size == 0) q->oldest = 0;

	// Updating the queue
	q->size++;
	q->td_ptrs[q->newest] = td;

	/*assert(q->size != MAX_NUM_TASKS, "Task queue should not overflow :)");

	//Modifying the queue
	q->size++;
	if(++(q->newest) == MAX_NUM_TASKS) q->newest = 0;
	q->td_ptrs[q->newest] = td;*/
}

Task_descriptor *dequeue_tqueue(Task_queue *q){
	assert(q->size != 0, "Task queue should have items to dequeue");

	//Modifying the queue
	q->size--;
	Task_descriptor *td = q->td_ptrs[q->oldest];
	if(++(q->oldest) == MAX_NUM_TASKS) q->oldest = 0;
	return td;
}
