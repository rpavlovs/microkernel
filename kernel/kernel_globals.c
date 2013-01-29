#include "kernel/kernel_globals.h"
#include "kernel/helpers.h"
#include "lib/bwio.h"

void enqueue_wqueue(Send_args *item, Wait_queue *queue){
	assert(queue->size == MAX_NUM_TASKS-1, "Wait queue should not overflow :)");

	//Modifying the queue
	queue->size++;
	if(++(queue->newest) == MAX_NUM_TASKS) queue->newest = 0;
	queue->args[queue->newest] = item;
}

Send_args *dequeue_wqueue(Wait_queue *queue){
	assert(queue->size == 0, "Wait queue should have items to dequeue");

	//Modifying the queue
	queue->size--;
	Send_args *item = queue->args[queue->oldest];
	if(++(queue->oldest) == MAX_NUM_TASKS) queue->oldest = 0;
	return item;
}

void enqueue_tqueue(Task_descriptor *td, Task_queue *q){
	assert(q->size == MAX_NUM_TASKS, "Task queue should not overflow :)");

	//Modifying the queue
	q->size++;
	if(++(q->newest) == MAX_NUM_TASKS) q->newest = 0;
	q->td_ptrs[q->newest] = td;
}

Task_descriptor *dequeue_tqueue(Task_queue *q){
	assert(q->size == 0, "Task queue should have items to dequeue");

	//Modifying the queue
	q->size--;
	Task_descriptor *td = q->td_ptrs[q->oldest];
	if(++(q->oldest) == MAX_NUM_TASKS) q->oldest = 0;
	return td;
}
