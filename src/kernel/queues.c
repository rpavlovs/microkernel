#include "kernelspace.h"

void enqueue_task(Task_descriptor *td, Schedule *sched){
	//Getting the priority queue
	Task_queue *q = &(sched->priority[td->priority]);

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

	//Enqueueing the task
	q->size++;
	q->td_ptrs[q->newest] = td;

	//Updating current_queue value
	if(sched->current_queue < td->priority)
	{
		sched->current_queue = td->priority;
	}
}

Task_descriptor *dequeue_task(Task_queue *q){
	// ASSERT: Verifying the size of the queue
	assert( q->size > 0, "Task queue should have items to dequeue" );

	// Dequeueing the task from the queue
	Task_descriptor *td = q->td_ptrs[q->oldest];
	if(++(q->oldest) >= SCHED_QUEUE_LENGTH) q->oldest = 0;

	// Updating the queue
	q->size--;

	// Updating current_queue value
	// TODO: Initialize current_queue in initializer
	// TODO: Check the current queue for more tasks
	// TODO: If empty, search lower priority queues for more tasks
	// TODO: If all queues are empty set current_queue = -1

	return td;
}
