#include "kernelspace.h"

void
init_scheduler( Kern_Globals *GLOBALS ) {	
	bwdebug( DBG_KERN, "init_scheduler: first task priority %d, address %d",
		FIRST_TASK_PRIORITY, (int) first_task );

	bwassert( FIRST_TASK_PRIORITY < SCHED_NUM_PRIORITIES &&
		FIRST_TASK_PRIORITY >= 0,
		"first task should have priority between 0 and %d", SCHED_NUM_PRIORITIES - 1 );

	//Initialize priority queues
	Scheduler *sched = &(GLOBALS->scheduler);
	int p;
	for( p = 0; p < SCHED_NUM_PRIORITIES; ++p )	{
		sched->queues[p].oldest = 0;
		sched->queues[p].newest = -1;	//TODO: maybe newest = -1?
		sched->queues[p].size = 0;
	}

	//Initialize the first task
	Task_descriptor *first_td = &(GLOBALS->tasks[0]);
	first_td->state = READY_TASK;
	first_td->priority = FIRST_TASK_PRIORITY;
	first_td->lr = (int *)first_task;
	
	//Updating the queue appropriately
	sched->queues[FIRST_TASK_PRIORITY].td_ptrs[0] = first_td;
	sched->queues[FIRST_TASK_PRIORITY].newest = 0;
	sched->queues[FIRST_TASK_PRIORITY].oldest = 0;
	sched->queues[FIRST_TASK_PRIORITY].size++;

	//Updating the schedule appropriately
	sched->last_issued_tid = 0;
	sched->tasks_alive = 1;
	sched->tasks_exited = 0;
	
	//Initializing hwi_watchers
	int i;
	for( i=0; i<HWI_NUM_EVENTS; i++ ) {
		sched->hwi_watchers[i] = 0;
	}
}

// Return:
// tid of the next task to run
int schedule( Kern_Globals * GLOBALS) {
	bwdebug( DBG_KERN, "SCHEDULE: entered. [last active: %d]",
		GLOBALS->scheduler.last_active_tid );

	int p = SCHED_NUM_PRIORITIES - 1;

	// If there are no tasks in all priority queues - PANIC
	for( ; GLOBALS->scheduler.queues[p].size == 0; --p ) {
		// if( p < 0 ) bwpanic( "SCHEDULE: pririty queues are all empty." );
		if( p < 0 ) {
			bwdebug( DBG_SYS, "SCHEDULE: WARNING: pririty queues are all empty." );
			break;
		}
	}

	// Get the non-empty queue with the highest priority
	Task_queue *queue = &(GLOBALS->scheduler.queues[p]);
	
	// TD of the task, which should run next
	Task_descriptor *next_td = queue->td_ptrs[queue->oldest];

	//Setting the last active task in the GLOBALS
	GLOBALS->scheduler.last_active_tid = next_td->tid;

	return next_td->tid;
}

// Start running the task with specified tid
// Return:
// interrupt ID of the first recieved interrupt
int activate( const int tid, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, "ACTIVATE: entered [activating task %d]", tid );

	Task_descriptor *td = &(GLOBALS->tasks[tid]);
	
	bwassert( td->state == READY_TASK, "It's only possible to activate a READY task. "
		"[task state: %d task_id: %d]", td->state, td->tid );
	
	td->state = ACTIVE_TASK;
	GLOBALS->scheduler.last_active_tid = tid;

	unsigned int uisp = (unsigned int) td->sp;
	unsigned int uilr = (unsigned int) td->lr;
	unsigned int uitd = (unsigned int) td;

	int request = execute_user_task(uisp, uilr, uitd);
	
	return request;
}

int getNextRequest( Kern_Globals *GLOBALS )
{
	bwdebug( DBG_KERN, "GET_NEXT_REQUEST: entered" );
	return activate( schedule( GLOBALS ), GLOBALS );
}

/////////////////////////////////////////////////////////////////////
//
// Utility functions
//
/////////////////////////////////////////////////////////////////////

int sched_get_free_tid( Kern_Globals *GLOBALS ) {
	Scheduler *sched = &(GLOBALS->scheduler);
	int new_tid;

	// Find a free task descriptor for a new task.
	new_tid = sched->last_issued_tid + 1;
	if( new_tid >= MAX_NUM_TASKS ) new_tid = 0;

	while( GLOBALS->tasks[new_tid].state != FREE_TASK ) {
		bwassert( ++new_tid < MAX_NUM_TASKS, 
			"sched_get_free_tid: Scheduler is out of task descriptors" );
		if( new_tid >= MAX_NUM_TASKS ){
			return -2;
		}
	}

	// Update schedule
	sched->last_issued_tid = new_tid;

	return new_tid;
}

void sched_add_td( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	// Utility variables
	Scheduler *sched;
	Task_queue *queue;

	// Initialize utility variables
	sched = &(GLOBALS->scheduler);
	queue = &(sched->queues[td->priority]);

	bwassert( queue->size < SCHED_QUEUE_LENGTH,
		"SYS_CREATE: Scheduler  queue must not be full" );

	// If the queue is empty or the newest pointer is at the end of the td_ptrs buffer
	// put the next td_ptr at the beginning on the buffer  
	if (queue->size == 0 || ++(queue->newest) >= SCHED_QUEUE_LENGTH) queue->newest = 0;
	
	// If the queue was empty then newest and oldest elements are the same 
	// and are at the beginning of the buffer
	if (queue->size == 0) queue->oldest = 0;

	queue->td_ptrs[queue->newest] = td;

	//Updating the schedule
	queue->size++;
	sched->tasks_alive++;
}

void sched_remove_td( Task_descriptor *td, Kern_Globals *GLOBALS ){
	// Utility variables
	Scheduler *sched;
	Task_queue *queue;

	// Initialize utility variables
	sched = &(GLOBALS->scheduler);
	queue = &(sched->queues[td->priority]);

	bwassert( queue->td_ptrs[queue->oldest] == td, "can only reschelude most recent task" );

	// Removing the first task from the corresponding queue
	if( ++(queue->oldest) >= SCHED_QUEUE_LENGTH ) queue->oldest = 0;

	// Updating the schedule
	queue->size--;
	sched->tasks_alive--;
}


















