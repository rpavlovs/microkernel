#include "kernelspace.h"

void
init_schedule( int first_task_priority, void (*first_task_code) ( ), Kern_Globals *GLOBALS ) {	
	debug( DBG_KERN, "INIT_SCHEDULE: first task priority %d, address %d",
		first_task_priority, (int) first_task_code );
	
	assert( first_task_priority < SCHED_NUM_PRIORITIES && first_task_priority >= 0,
		"first task should have priority between 0 and %d", SCHED_NUM_PRIORITIES - 1 );

	Scheduler *sched = &(GLOBALS->scheduler);
	int p;
	for( p = 0; p < SCHED_NUM_PRIORITIES; ++p )	{
		sched->queues[p].oldest = 0;
		sched->queues[p].newest = 0;
		sched->queues[p].size = 0;
	}

	Task_descriptor *first_td = &(GLOBALS->tasks[0]);
	first_td->state = READY_TASK;

	//Setting priority of the task
	first_td->priority = first_task_priority;
	//Setting link register to the address of task code
	first_td->lr = (int *)first_task_code;
	//Setting the next instruction
	// first_td->next_instruction = (unsigned int) first_task_code + ELF_START;
	
	//Updating the queue appropriately////////////////////////
	sched->queues[first_task_priority].td_ptrs[0] = first_td;
	sched->queues[first_task_priority].newest = 0;
	sched->queues[first_task_priority].oldest = 0;
	sched->queues[first_task_priority].size++;

	//Updating the schedule appropriately
	sched->last_issued_tid = 0;
	sched->tasks_alive = 1;
	
	//Initializing hwi_watchers
	int i;
	for(i=0; i<HWI_NUM_EVENTS; i++)
	{
		sched->hwi_watchers[i] = 0;
	}
}

// Return:
// tid of the next task to run
int schedule( Kern_Globals * GLOBALS) {
	debug( DBG_KERN, "SCHEDULE: entered. [last active: %d]",
		GLOBALS->scheduler.last_active_tid );

	int p = SCHED_NUM_PRIORITIES - 1;

	// If there are no tasks in all priority queues - PANIC
	for( ; GLOBALS->scheduler.queues[p].size == 0; --p ) {
		// if( p < 0 ) panic( "SCHEDULE: pririty queues are all empty." );
		if( p < 0 ) {
			debug( DBG_SYS, "SCHEDULE: WARNING: pririty queues are all empty." );
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
	debug( DBG_KERN, "ACTIVATE: entered [activating task %d]", tid );

	Task_descriptor *td = &(GLOBALS->tasks[tid]);

	if( td->state != READY_TASK ) {
		int i = 0;
		for( ; i < SCHED_NUM_PRIORITIES; ++i ) {
			Task_queue *queue = &(GLOBALS->scheduler.queues[i]);
			bwprintf( COM2, "SCHEDULE: p[%d].size: %d", i,
				queue->size );
			if( queue->size != 0 ) {
				bwprintf( COM2, " oldest: %d newest: %d\n",
				queue->td_ptrs[queue->oldest]->tid,
				queue->td_ptrs[queue->newest]->tid );
			} else {
				bwprintf( COM2, "\n");
			}
		}
	}
	
	assert( td->state == READY_TASK, "It's only possible to activate a READY task. "
		"[task state: %d]", td->state );
	
	td->state = ACTIVE_TASK;

	unsigned int uisp = (unsigned int) td->sp;
	unsigned int uilr = (unsigned int) td->lr;
	unsigned int uitd = (unsigned int) td;

	int request = execute_user_task(uisp, uilr, uitd);
	
	return request;
}

int getNextRequest( Kern_Globals *GLOBALS )
{
	debug( DBG_KERN, "GET_NEXT_REQUEST: entered" );
	return activate( schedule( GLOBALS ), GLOBALS );
}


