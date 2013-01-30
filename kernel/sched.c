#include "kernel/sched.h"
#include "kernel/kernel_globals.h"
#include "lib/bwio.h"
#include "kernel/helpers.h"
#include "config/mem_init.h"
#include "kernel/syscall.h"
#include "kernel/sysfuncs.h"

extern int execute_user_task(unsigned int userTaskSP, unsigned int nextInstruction, unsigned int taskID);

__asm__(
	/*
		execute_user_task: 
		This method should be executed whenever the kernel wants to return control
		to an user task. It performs the opposite task to the swi_main_handler. 
		Parameters:
		- The SP of the user task.  
		- The next instruction to execute in the user task.
		- The TID of the current ( active ) user task. 
	*/

	"\n"
	"execute_user_task:"												"\n\t"

	// Store the information about the kernel as would happen in a normal task. 
	"MOV	ip, sp"														"\n\t"
	"STMFD	sp!, { fp, ip, lr, pc }"									"\n\t"
	"SUB	fp, ip, #4"													"\n\t"

	// Store the kernel state.
	"SUB	sp, sp, #4"													"\n\t"

	// Store the TID
	"STR	r2, [ sp, #0 ]"												"\n\t"
	"STMFD	sp!, { r4-r11 }"											"\n\t"

	// Store information temporarily
	"SUB	sp, sp, #4"													"\n\t"

	// The task's SPSR is loaded.
	"LDR	r3, [ r0, #14*4 ]"											"\n\t"

	// The next instruction to execute in the user task.
	"STR	r1, [ r0, #14*4 ]"											"\n\t"

	// The task's SPSR		
	"STR	r3, [ sp, #0 ]"												"\n\t"

	// Load the state of the task. 
	// 
	// Switch to system mode.
	"MSR	cpsr_c, #0x1F"												"\n\t" 
	"MOV	sp, r0"														"\n\t"			
	"LDMFD	sp!, { r0-r12, lr }"										"\n\t"
	
	// Temporarily store the r0 (to be able to use this registry).
	"STR	r0, [ sp, #-4 ]"											"\n\t"

	// Return to supervisor mode.  
	"MSR	cpsr_c, #0x13"												"\n\t"

	// Pass control to the user task.
	// 
	// Loads the previously stored SPSR. 
	"LDR	r0, [ sp, #0 ]"												"\n\t"
	
	// "Remove" the top elements of the stack that won't be used anymore.
	// (Kernel's stack).	  
	"ADD	sp, sp, #4"													"\n\t"
	
	// Switch to user mode.			
	"MSR	cpsr, r0"													"\n\t"

	// Remove the temporarily stored r0. 
	"LDR	r0, [ sp, #-4 ]"											"\n\t"

	// Remove the stored "counter"
	"ADD	sp, sp, #4"													"\n\t"

	// Jump to the next instruction in the user task.
	"LDR	PC, [ sp, #-4 ]"											"\n\t"

);


void RetrieveSysCallArgs( int *sysCallArguments, int numArguments, unsigned int taskSP )
{
	// bwprintf( COM2, "RetrieveSysCallArgs: ENTERED\n");

	// TODO: Add an assertion here.

	// The arguments are stored in the memory addresses that hold R0-R3 of the user task. 
	// If there are more arguments they are stored in the user task's stack. 	
	int *ptr = ( int * ) taskSP; 

	// // DEBUGGING
	// int j; 
	// for ( j = -1; j < 30; j++ )
	// {
	// 	bwprintf( COM2, "DEB VALUES. Index:%d Stack value: %d\n", j, *(ptr + j) );
	// }
	
	int i; 
	for ( i = 0; i < numArguments && i < MAX_NUM_ARGUMENTS ; i++ )
	{
		// Copy the arguments.
		*( sysCallArguments++ ) = *( ptr++ ); 

		// The last register that holds arguments. The next place to look arguments is the normal user task stack. 
		if ( i == 3 ) 
		{
			//WHITE MAGIC
			ptr += 20;
		}
	}
}

void SetSysCallReturn( int returnValue, unsigned int taskSP )
{
	// bwprintf( COM2, "SetSysCallReturn: ENTERED\n");

	// The return value is in the address that currently holds R0 for the task. 
	int *ptr = ( int * ) taskSP; 
	//ptr += 13; 	// Position the pointer at the R0 address.
	// TODO: Set this as a constant. 

	*( ptr ) = returnValue; 
}

void handle_request( int request, Kern_Globals *GLOBALS ) 
{

	// Create a placeholder for the arguments.
	int sysCallArguments[ MAX_NUM_ARGUMENTS ];

	Task_descriptor *td = &( GLOBALS->tasks[ GLOBALS->schedule.last_active_tid ]);
	
	int returnValue;
	unsigned int taskSP =  ( unsigned int ) td->sp;

	switch ( request )
	{
		case CREATE_SYSCALL:
			RetrieveSysCallArgs( sysCallArguments, CREATE_ARGS, taskSP);
			
			returnValue = sys_create( sysCallArguments[0],
								(void *) sysCallArguments[1],
								td, GLOBALS);
			
			SetSysCallReturn(returnValue, taskSP);
			
			debug( "CREATE_SYSCALL handled" );

			break;

		case MYTID_SYSCALL:
			returnValue = sys_mytid(td, GLOBALS);
			
			SetSysCallReturn(returnValue, taskSP);

			debug( "MYTID_SYSCALL handled" );

			break;
		case MYPARENTTID_SYSCALL:
			returnValue = sys_myparenttid(td, GLOBALS);
			SetSysCallReturn(returnValue, taskSP);

			debug( "MYPARENTTID_SYSCALL handled" );

			break;
		case PASS_SYSCALL:
			sys_pass(td, GLOBALS);

			debug( "PASS_SYSCALL handled" );

			break;
		case EXIT_SYSCALL:

			sys_exit(td, GLOBALS);

			debug( "EXIT_SYSCALL handled" );
			
			break;

		case TESTCALL_SYSCALL:
			RetrieveSysCallArgs( sysCallArguments, TESTCALL_ARGS, taskSP);
			returnValue = sys_testcall(sysCallArguments[0], sysCallArguments[1], sysCallArguments[2],
						   sysCallArguments[3], sysCallArguments[4], sysCallArguments[5]);
			SetSysCallReturn(returnValue, taskSP);
	
			break;

		case SEND_SYSCALL:
			RetrieveSysCallArgs( sysCallArguments, SEND_ARGS, taskSP);
			returnValue = sys_send(
							sysCallArguments[0], 
							sysCallArguments[1], 
							sysCallArguments[2],
						   	sysCallArguments[3], 
						   	sysCallArguments[4],
						   	td, GLOBALS);
			SetSysCallReturn(returnValue, taskSP);

			break;

		case RECEIVE_SYSCALL:
			RetrieveSysCallArgs( sysCallArguments, RECEIVE_ARGS, taskSP);
			returnValue = sys_receive(
							sysCallArguments[0], 
							sysCallArguments[1], 
							sysCallArguments[2],
							td, GLOBALS);
			SetSysCallReturn(returnValue, taskSP);

			break;

		case REPLY_SYSCALL:
			RetrieveSysCallArgs( sysCallArguments, REPLY_ARGS, taskSP);
			returnValue = sys_reply(
							sysCallArguments[0], 
							sysCallArguments[1], 
							sysCallArguments[2],
							td, GLOBALS);
			SetSysCallReturn(returnValue, taskSP);

			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Initialize Schedule struct
void init_schedule( int first_task_priority, void (*first_task_code) ( ), Kern_Globals *GLOBALS )
{	
	//Verifying that the task priority is correct
	assert(first_task_priority < SCHED_NUM_PRIORITIES && first_task_priority >= 0,
		"first task should have priority between 0 and 16" );

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

	// Deleting the task from the scheduler
	//if( ++(queue->oldest) >= SCHED_QUEUE_LENGTH ) queue->oldest = 0;
	//--(queue->size);

	/*//If there are more than one task in the queue
	if(queue->size > 1)
	{
		// Removing the first task from the queue
		if (++(queue->oldest) >= SCHED_QUEUE_LENGTH) queue->oldest = 0;

		// Updating the task's state
		next_td->state = READY_TASK;

		// Adding the task to the end of the queue
		if (++(queue->newest) >= SCHED_QUEUE_LENGTH) queue->newest = 0;
		queue->td_ptrs[queue->newest] = next_td;
	}*/

	//Setting the last active task in the GLOBALS
	GLOBALS->schedule.last_active_tid = next_td->tid;

	return next_td->tid;
}

// Start running the task with specified tid
// Return:
// interrupt ID of the first recieved interrupt
int activate( const int tid, Kern_Globals *GLOBALS ) {
	
	Task_descriptor *td = &(GLOBALS->tasks[tid]);

	assert( td->state == READY_TASK, "It should only be possible to activate a READY task" );
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
	return activate( schedule( GLOBALS ), GLOBALS );
}





























