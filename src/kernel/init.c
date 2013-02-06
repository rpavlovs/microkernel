#include "kernelspace.h"

void init_message_queues( Kern_Globals *GLOBALS ) {
	int tid ;
	for( tid = 0; tid < MAX_NUM_TASKS; ++tid) {
		Message_queue *receive_queue = &(GLOBALS->tasks[tid].receive_queue);
		receive_queue->newest = -1;
		receive_queue->oldest = 0;
		receive_queue->size = 0;
	}
}

// NOTE: Stacks grow downwards, with sp pointing to the empty spot.
void init_task_descriptors( Kern_Globals *GLOBALS ) {
	int tid;
	for( tid = 0; tid < MAX_NUM_TASKS; tid++)
	{
		Task_descriptor *td = &( GLOBALS->tasks[tid] );
		// Setting the Task ID
		td->tid = tid;
		// Setting stack pointer to give each task the same address space
		td->sp = (int *)( TASKS_MEMORY_START - (tid * TASKS_MEMORY_PER_INSTANCE) - 3 );
		// Setting function pointer
		td->fp = td->sp;
		
		// Initializing the stack
		int *temp_sp = td->sp;

		// SPSR in the user mode with everything turned off
		*temp_sp = 0x10;
		temp_sp--;
		
		// Assigning random values to the registers
		int i;
		for( i=0; i <= 10 ; i++ ) {
			*temp_sp = i;
			temp_sp--;
		}
		
		// Function pointer
		*temp_sp = *(td->fp);

		temp_sp--;

		*temp_sp = 11;

		temp_sp--;

		*temp_sp = 0;

		td->sp = temp_sp;

		// Setting the state of the task
		td->state = FREE_TASK;
	}
}

void init_hardware() {
	
	// Start Debug timer (Timer 4)
    int *hi = (int *)Timer4ValueHigh;
    *hi = (1 << 8);
}

void initialize( Kern_Globals *GLOBALS ) {

	initialize_context_switching(); 
	
	init_hardware();

    initialize_context_switching(); 

	init_message_queues( GLOBALS );

	init_task_descriptors( GLOBALS );

	init_schedule( 15, first_task, GLOBALS );
}
