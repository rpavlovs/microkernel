#include "kernel/kernel_globals.h"
#include "kernel/helpers.h"
#include "lib/bwio.h"

void enqueue_wqueue(int item, Wait_queue *queue){
	assert(queue->size < MAX_NUM_TASKS-1, "Wait queue should not overflow :)");

	//Modifying the queue
	queue->size++;
	if(++(queue->newest) == MAX_NUM_TASKS) queue->newest = 0;
	queue->tids[queue->newest] = item;
}

int dequeue_wqueue(Wait_queue *queue){
	assert(queue->size == 0, "The queue should have items to dequeue");

	//Modifying the queue
	queue->size--;
	int item = queue->tids[queue->oldest];
	if(++(queue->oldest) == MAX_NUM_TASKS) queue->oldest = 0;
	return item;
}
