#include "kernel/kernel_globals.h"
#include "kernel/sysfuncs.h"
#include "lib/bwio.h"

int sys_create( int priority, void (*code) ( ), Kern_Globals *GLOBALS ) {
	// ERROR: Scheduler was given a wrong task priority.
	if( priority < 0 || priority >= SCHED_NUM_PRIORITIES ) return -1;
	
	// Getting the schedule
	Schedule *sched = &(GLOBALS->schedule);
	int new_tid;
	Task_descriptor *new_td;

	// Find a free task descriptor for a new task.
	new_tid = sched->last_issued_tid + 1;
	if( new_tid >= MAX_NUM_TASKS ) new_tid = 0;
	while( GLOBALS->tasks[new_tid].state != FREE_TASK ) {
		// ERROR: Scheduler is out of task descriptors. 
		if( ++new_tid >= MAX_NUM_TASKS ) return -2;
	}

	//Updating the schedule
	sched->last_issued_tid = new_tid;
	
	// Setup new task descriptor
	new_td = &(GLOBALS->tasks[new_tid]);
	new_td->state = READY_TASK;
	new_td->priority = priority;
	new_td->lr = (int *)code;

	// Add new task descriptor to a proper scheduler queue
	Task_queue *queue = &(sched->priority[priority]);

	// ASSERT: Verifying the size of the queue
	assert( queue->size < SCHED_QUEUE_LENGTH, "Scheduler queue must not be full" );

	// If the queue is empty or the newest pointer is at the end of the td_ptrs buffer
	// put the next td_ptr at the beginning on the buffer  
	if (queue->size == 0 || ++(queue->newest) >= SCHED_QUEUE_LENGTH) queue->newest = 0;
	
	// If the queue was empty then newest and oldest elements are the same 
	// and are at the beginning of the buffer
	if (queue->size == 0) queue->oldest = 0;

	// Updating the queue
	queue->size++;
	queue->td_ptrs[queue->newest] = new_td;

	return new_tid;
}

int sys_mytid(Task_descriptor *td, Kern_Globals *GLOBALS )
{
	sys_reschedule(td, GLOBALS);
	return td->tid;
}

int sys_myparenttid(Task_descriptor *td, Kern_Globals *GLOBALS )
{
	sys_reschedule(td, GLOBALS);
	return td->parent_tid;
}

void sys_pass(Task_descriptor *td, Kern_Globals *GLOBALS ) {
	sys_reschedule(td, GLOBALS);
}

void sys_exit(Task_descriptor *td, Kern_Globals *GLOBALS ) {
	// Getting task properties
	int tid = td->tid;
	int priority = td->priority;
	
	// Getting the schedule
	Schedule *sched = &(GLOBALS->schedule);
	Task_queue *pqueue = &(sched->priority[priority]);

	// Removing the first task from the queue
	if (++(queue->oldest) >= SCHED_QUEUE_LENGTH) queue->oldest = 0;

	// Updating the task's state
	td->state = ZOMBIE_TASK;

	// Updating the queue
	(td->size)--;
}

void sys_reschedule(Task_descriptor *td, Kern_Globals *GLOBALS ){
	// If the task is alone - do nothing
	// Else enqueue -dequeue

	// Getting task properties
	int tid = td->tid;
	int priority = td->priority;
	
	// Getting the schedule
	Schedule *sched = &(GLOBALS->schedule);
	Task_queue *pqueue = &(sched->priority[priority]);

	//If there are more than one task in the queue
	if(pqueue->size > 1)
	{
		// Removing the first task from the queue
		if (++(queue->oldest) >= SCHED_QUEUE_LENGTH) queue->oldest = 0;

		// Updating the task's state
		td->state = READY_TASK;

		// Adding the task to the end of the queue
		(queue->newest)++;
		queue->td_ptrs[queue->newest] = td;
	}
}



























