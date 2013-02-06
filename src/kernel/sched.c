#include "kernelspace.h"

// Initialize Schedule struct
void init_schedule( int first_task_priority, void (*first_task_code) ( ), Kern_Globals *GLOBALS )
{	
	//Verifying that the task priority is correct
	assert( first_task_priority < SCHED_NUM_PRIORITIES && first_task_priority >= 0,
		"first task should have priority between 0 and 16" );

	debug( DBG_KERN, "INIT_SCHEDULE: first task priority %d, address %d",
		first_task_priority, (int) first_task_code );

	//Getting the schedule
	Schedule *sched = &(GLOBALS->schedule);

	//Initializing all priority queues
	int p;
	for (p = 0; p < SCHED_NUM_PRIORITIES; ++p)
	{
		sched->priority[p].oldest = 0;
		sched->priority[p].newest = 0;
		sched->priority[p].size = 0;
	}

	//Adding the first task to the queue//////////////////////
	//Getting task descriptor
	Task_descriptor *first_td = &(GLOBALS->tasks[0]);
	//Setting task state to READY
	first_td->state = READY_TASK;

	//Setting priority of the task
	first_td->priority = first_task_priority;
	//Setting link register to the address of task code
	first_td->lr = (int *)first_task_code;
	//Setting the next instruction
	// first_td->next_instruction = (unsigned int) first_task_code + ELF_START;
	
	//Updating the queue appropriately////////////////////////
	sched->priority[first_task_priority].td_ptrs[0] = first_td;
	sched->priority[first_task_priority].newest = 0;
	sched->priority[first_task_priority].oldest = 0;
	sched->priority[first_task_priority].size++;

	//Updating the schedule appropriately
	sched->last_issued_tid = 0;
	sched->tasks_alive = 1;
}

// Return:
// tid of the next task to run
int schedule( Kern_Globals * GLOBALS) {
	debug( DBG_KERN, "SCHEDULE: entered" );

	int p = SCHED_NUM_PRIORITIES - 1;

	// If there are no tasks in all priority queues - PANIC
	while( GLOBALS->schedule.priority[p].size == 0 ) {
		--p;
		if( p < 0 ) panic( "Scheduler has nothing in it's queues." );
	}

	// Get the non-empty queue with the highest priority
	Task_queue *queue = &(GLOBALS->schedule.priority[p]);
	
	// TD of the task, which should run next
	Task_descriptor *next_td = queue->td_ptrs[queue->oldest];

	//Setting the last active task in the GLOBALS
	GLOBALS->schedule.last_active_tid = next_td->tid;

	return next_td->tid;
}

// Start running the task with specified tid
// Return:
// interrupt ID of the first recieved interrupt
int activate( const int tid, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "ACTIVATE: entered [activating task %d]", tid );

	Task_descriptor *td = &(GLOBALS->tasks[tid]);

	assert( td->state == READY_TASK, "It's only possible to activate a READY task. "
		"[task state: %d]", td->state );
	td->state = ACTIVE_TASK;

	unsigned int uisp = (unsigned int) td->sp;
	unsigned int uilr = (unsigned int) td->lr;
	unsigned int uitd = (unsigned int) td;

	//Executing using link register
	int request = execute_user_task(uisp, uilr, uitd);

	return request;
}

int getNextRequest( Kern_Globals *GLOBALS )
{
	debug( DBG_KERN, "GET_NEXT_REQUEST: entered" );
	return activate( schedule( GLOBALS ), GLOBALS );
}


