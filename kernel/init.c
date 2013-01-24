#include "kernel/kernel_globals.h"
#include "kernel/init.h"
#include "kernel/sched.h"
#include "config/ts7200.h"
#include "config/mem_init.h"
#include "tasks/first.h"
#include "lib/bwio.h"

// NOTE: Stacks grow downwards, with sp pointing to the empty spot;
void init_task_descriptors( Kern_Globals *KERN_GLOBALS ) {
	// Task ID
	int tid;

	//DEBUGGING
	bwprintf( COM2, "Started initializing task descriptors...\n\r");

	for( tid = 0; tid < MAX_NUM_TASKS; tid++) 
	{
		Task_descriptor *td = &( KERN_GLOBALS->tasks[tid] );
		// Setting the Task ID
		td->tid = tid;
		// Setting stack pointer to give each task the same address space
		td->sp = (int *)( TASKS_MEMORY_START - (tid * TASKS_MEMORY_PER_INSTANCE) - 3 );		// DIRTY HACK!!!
		// Setting function pointer
		td->fp = td->sp;		
		
		// Initializing the stack////////////////////////////////////////////////
		int *temp_sp = td->sp;
		temp_sp--;

		//DEBUGGING
		bwprintf( COM2, "Temp stack: %x\n\r", temp_sp);

		// SPSR in the user mode with everything turned off
		*temp_sp = 0x10;
		//DEBUGGING
		bwprintf( COM2, "Temp stack assignment: %x\n\r", *temp_sp);
		temp_sp--;
		
		// Assigning random values to the registers
		int i;
		for(i=0; i < 10 ; i++){
			*temp_sp = i;

			//DEBUGGING
			bwprintf( COM2, "Temp stack random assignment: %x\n\r", *temp_sp);

			temp_sp--;
		}
		
		// Function pointer
		*temp_sp = *(td->fp);

		//DEBUGGING
		bwprintf( COM2, "Temp stack FP assignment: %x\n\r", *temp_sp);

		temp_sp--;

		*temp_sp = 11;

		//DEBUGGING
		bwprintf( COM2, "Temp stack assignment 11: %x\n\r", *temp_sp);

		temp_sp--;

		*temp_sp = 0;

		//DEBUGGING
		bwprintf( COM2, "Temp stack assignment sp: %x\n\r", *temp_sp);

		//CHECK THIS LATER... JUST IN CASE
		td->sp = temp_sp;

		// Setting the state of the task
		td->state = FREE_TASK;
	}

	//DEBUGGING
	bwprintf( COM2, "Finishing initializing task descriptors...\n\r");

}

void initialize( Kern_Globals *KERN_GLOBALS ) {
	//Where is the kernel entry?
	//Turn off interrupts in ICU!
	//Prepare the kernel data structures
		//Where is the kernel stack pointer right now? (Kernel stack pointer is in the end of the RAM memory)
		//What does the stack look like? (Stack goes downwards from the end of the RAM memory)
		//Do we want it there? YES
		//Do we want it somewhere else? NO
		//This is the last chance to change it! OK :)
		//If we are going to change it - it is a good idea to keep changed stuff around! WHATEVER :)
	//Kernel data structures
		//An array of empty ready queues
		//A pointer to the TD of the active task
		//An array of TDs
		//A free list of pointers to free TDs
	//Prepare memory to be used by tasks
		//Allocate task memory
	//Create first user task
		//Can run with interrupts turned off by now
	//NOTE: the place where the kernel starts executing has the global name main which cannot be reused



	//Where is the kernel stack pointer right now?
	bwprintf( COM2, "Kernel stack pointer: " );
	asm (
			"mov r0, #1"	"\n\t"
			"mov r1, sp"	"\n\t"
			"bl bwputr"		"\n\t"
		);
	bwprintf( COM2, "\n\r" );

	init_task_descriptors( KERN_GLOBALS );

	//DEBUGGING
	bwprintf( COM2, "Task descriptors are initialized.\n\r" );

	init_schedule( 8, first_task, KERN_GLOBALS );

	//DEBUGGING
	bwprintf( COM2, "Schedule is initialized\n\r" );

	//DEBUGGING
	bwprintf( COM2, "Initialization is complete." );
}
