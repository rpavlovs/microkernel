#include "kernel/sched.h"
#include "lib/bwio.h"

// Initialize Schedule struct
void init_schedule( Schedule *sched ) {
	int i;
	for (i = 0; i < SCHED_NUM_PRIORITIES; ++i)
	{
		//Resetting all priority queues
		sched->priority[i]->oldest = 0;
		sched->priority[i]->newest = 0;
		sched->priority[i]->size = 0;
	}
	sched->latest_tid = 0;
}

// Add task to scheduler as ready to run
int add_task( int priority, void (*code) ( ), Schedule *sched ) {
	
	//Checking for errors
	if( sched->priority[priority]->size > SCHED_QUEUE_MAX_LENGTH ) {
		bwprintf( COM2, "ERROR: Scheduler round buffer overflow.");
		return -3;
	}

	if( sched->latest_tid >= SCHED_TID_MAX_VAL ) {
		bwprintf( COM2, "ERROR: Scheduler is out of task descriptors.");
		return -2;
	}
	
	if( priority < 0 || priority >= SCHED_NUM_PRIORITIES ) {
		bwprintf( COM2, "ERROR: Scheduler was given a wrong task priority.");
		return -1;
	}

	//Add a task to a priority queue
	if( sched->priority[priority]->size == 0 ) {
		//Getting address of the queue, which is also address of the first element
		Task *new_proc = sched->priority[priority]->queue;
		//Updating priority queue
		sched->priority[priority]->newest = new_proc;
		sched->priority[priority]->oldest = new_proc;
		sched->priority[priority]->size = 1;
		new_proc->tid = sched->latest_tid++;
		return new_proc->tid;
	}

	Task *new_proc = sched->priority[priority]->newest + 1;
	//If the end of the buffer is reached, start from the begining
	if( new_proc >= sched->priority[priority]->queue + SCHED_QUEUE_MAX_LENGTH ) {
		new_proc = sched->priority[priority]->queue;
	}
	
	//Updating priority queue
	sched->priority[priority]->newest = new_proc;
	sched->priority[priority]->size++;
	new_proc->tid = sched->latest_tid++;

	return new_proc->tid;
}

//Why do we need this function? Don't we have it in syscall.c?
int parent_tid_syscall( Schedule *sched ) {
	return 0;
}

int activate( int tid ) {
	return 0;
}

//Why do we need this function? Don't we have it in syscall.c?
void pass_syscall( ) {

}


int schedule( Schedule *sched ) {
	return 0;
}


int getNextRequest( Schedule *sched ) {
  return activate( schedule( sched ) ); //the active task doesn't change
}
