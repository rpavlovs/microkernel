#include "kernel/kernel_globals.h"
#include "kernel/init.h"
#include "kernel/sched.h"
#include "config/ts7200.h"
#include "config/mem_init.h"
#include "tasks/first.h"
#include "lib/bwio.h"

#define SWI_VECTOR 0x8

extern void swi_main_handler();

asm (
	/*
		This handler is executed whenever there's a software interrupt. 
		The processor automatically leaves the system in the following state. 
		1- Copies the current CPSR (Current Program Status Register) into 
		   the SPSR_SVC (Saved Program Status Register->Supervisor)
		2- Sets the CPSR mode bits to supervisor mode (takes the processor into SVC 
		   mode). 
		3- Sets the CPSR IRQ to disable -> disables the interrupts. 
		4- Stores the value (PC + 4) into LR_SVC. This means that the LR that the 
		   supervisor receives is pointing to the next address in the user program. 
		5- Forces the PC to 0x08 (the default SWI handler). This address is in the
		   vector table (spring board).  
	*/

	"\n"

	"swi_main_handler:"													"\n\t"
	// Save in the stack the arguments (in the registers) since they might get
	// erased. 
	// NOTE: The current SP is the SP_SVC, so the user SP is not affected.
	//       
	// Switch to system mode. 
	"MSR	CPSR_c, #0x1F"												"\n\t"

	// Leave space for SPSR. 	 
	"SUB	sp, sp, #4"														"\n\t"

	// Store all the registers (except 13-sp- and 15-pc-).		
	"STMFD	sp!, { r0-r12, lr }"								"\n\t"

	// Store the task's SP so that it can be stored later into the TR.
	"MOV	r0, sp"																"\n\t"

	// Return to supervisor mode. 
	"MSR	cpsr_c, #0x13"												"\n\t"

	// Get the value of the system call
	// 
	// Store in the task's stack the spsr (the original CPSR of the task).
	"MRS	r1, spsr"															"\n\t"			 
	"STR	r1, [ r0, #14*4 ]"										"\n\t"

	// Store the return value in the r1 so that it is stored later into the TR. 
	"MOV	r1, lr"																"\n\t"		

	// Restore the kernel state
	"LDMFD	sp!, { r4-r11 }"										"\n\t"

	/*// Execute the C system call handler
	// Store the next instruction to run in the r1 so that it is stored later
	// into the TR. 
	"LDR	r2, [ lr, #-4 ]\n\t"		
	"BIC	r2, r2, #0xff000000\n\t"	
	"LDR	r3, [ sp, #0 ]\n\t"		// The pointer to the active TD is loaded into
	register 3.
	"BL	ExecuteCSWIHandler\n\t"*/

	// Execute the C system call handler.
	// The pointer to the active TD is loaded into register 2.
	"LDR	r2, [ sp, #0 ]"												"\n\t"		

	// Store the next instruction to run in the r1 so that it is stored later
	// into the TR. 
	"LDR	r3, [ lr, #-4 ]"											"\n\t"		
	"BIC	r3, r3, #0xff000000"									"\n\t"

	// This variable is stored in local memory to prevent it from being deleted
	// by the function call. 
	"STR	r3, [ sp, #0 ]"												"\n\t"

	// r0 - task stack pointer
	// r1 - next address
	// r2 - task descriptor pointer	
	"BL	ExecuteCSWIHandler"											"\n\t"

	// Set the return value.
	"LDR	r0, [ sp, #0 ]"												"\n\t"	
	
	// Return control to the kernel C code. 
	"sub	sp, fp, #12"													"\n\t"
	"LDMFD	sp, { fp, sp, pc }"									"\n\t"
);

void ExecuteCSWIHandler( unsigned int taskSP, unsigned int lr,
													unsigned int activeTD )
{

	//bwprintf( COM2, "kerxit.c: Updating Active Task   TaskSP: %d 
	//				ReturnValue: %d    SWIValue: %d    activeTID: %d .\n\r",
	//   		taskSP, nextAddress, activeTD );

	// Update the task descriptor
	Task_descriptor *td = ( Task_descriptor * ) activeTD;
	td->sp = (int *) taskSP;
	td->lr = (int *) lr;

}

//////////////////////////////////////////////////////////////////////////////
//
//Initializations
//
//////////////////////////////////////////////////////////////////////////////

//Executed once during initialization
int installSwiHandler( unsigned int handlerLoc, unsigned int *vector )
{
	unsigned int vec; 

	vec = ( ( handlerLoc - ( unsigned int ) vector - 0x8 ) >> 2 );
	if ( vec & 0xFF000000 )
	{
		return 1; // There was a problem 
	}

	vec = 0xEA000000 | vec;
	*vector = vec; 

	return 0; 
}

void init_wait_queues( Kern_Globals *KERN_GLOBALS ){

	int i;
	for(i=0;i<MAX_NUM_TASKS;i++){
		KERN_GLOBALS->wqueues[i].newest = -1;
		KERN_GLOBALS->wqueues[i].oldest = 0;
		KERN_GLOBALS->wqueues[i].size = 0;
	}

}

// NOTE: Stacks grow downwards, with sp pointing to the empty spot;
void init_task_descriptors( Kern_Globals *KERN_GLOBALS ) {
	// Task ID
	int tid;

	//DEBUGGING
	//bwprintf( COM2, "Started initializing task descriptors...\n\r");

	for( tid = 0; tid < MAX_NUM_TASKS; tid++) 
	{
		Task_descriptor *td = &( KERN_GLOBALS->tasks[tid] );
		// Setting the Task ID
		td->tid = tid;
		// Setting stack pointer to give each task the same address space
		td->sp = (int *)( TASKS_MEMORY_START - (tid * TASKS_MEMORY_PER_INSTANCE) - 3 );
		// Setting function pointer
		td->fp = td->sp;
		
		// Initializing the stack
		int *temp_sp = td->sp;
		// temp_sp--;

		// bwprintf( COM2, "Temp stack: %x\n\r", temp_sp);

		// SPSR in the user mode with everything turned off
		*temp_sp = 0x10;
		/*//DEBUGGING
		bwprintf( COM2, "Temp stack assignment: %x\n\r", *temp_sp);*/
		temp_sp--;
		
		// Assigning random values to the registers
		int i;
		for(i=0; i <= 10 ; i++){
			*temp_sp = i;

			/*//DEBUGGING
			bwprintf( COM2, "Temp stack random assignment: %x\n\r", *temp_sp);*/

			temp_sp--;
		}
		
		// Function pointer
		*temp_sp = *(td->fp);

		/*//DEBUGGING
		bwprintf( COM2, "Temp stack FP assignment: %x\n\r", *temp_sp);*/

		temp_sp--;

		*temp_sp = 11;

		/*//DEBUGGING
		bwprintf( COM2, "Temp stack assignment 11: %x\n\r", *temp_sp);*/

		temp_sp--;

		*temp_sp = 0;

		/*//DEBUGGING
		bwprintf( COM2, "Temp stack assignment sp: %x\n\r", *temp_sp);*/

		//DEBUGGING
		/*if(tid==0){

			bwprintf( COM2, "Showing the first task's stack.\n\r");

			int *j;
			for(j=(td->sp); j > (temp_sp - 1); j--){
				bwprintf( COM2, "Value: %d\n\r", *j);
			}

			bwprintf( COM2, "Showing the first task's stack.\n\r");
		}*/

		//CHECK THIS LATER... JUST IN CASE
		td->sp = temp_sp;

		// Setting the state of the task
		td->state = FREE_TASK;

		// Setting the wait queue pointer
		td->receive_queue = &(KERN_GLOBALS->wqueues[tid]);
	}

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

	//bwsetfifo( COM2, OFF );

	//Where is the kernel stack pointer right now?
	/*bwprintf( COM2, "Kernel stack pointer: " );
	asm (
			"mov r0, #1"	"\n\t"
			"mov r1, sp"	"\n\t"
			"bl bwputr"		"\n\t"
		);
	bwprintf( COM2, "\n\r" );*/

	installSwiHandler((unsigned int) swi_main_handler, (unsigned int *) SWI_VECTOR);

	init_wait_queues( KERN_GLOBALS );

	init_task_descriptors( KERN_GLOBALS );

	//init_schedule( 8, first_task, KERN_GLOBALS );
	init_schedule( 8, test_task, KERN_GLOBALS );
}

















