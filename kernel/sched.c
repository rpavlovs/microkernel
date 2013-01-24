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
		This method should be executed whenever the kernel wants to return control to an user task. 
		It performs the opposite task to the swi_main_handler. 
		Parameters:
		- The SP of the user task.  
		- The next instruction to execute in the user task.
		- The TID of the current ( active ) user task. 
	*/
	"\nexecute_user_task:\n\t"

	// Store the information about the kernel as would happen in a normal task. 
	"MOV	ip, sp\n\t"
	"STMFD	sp!, { fp, ip, lr, pc }\n\t"
	"SUB	fp, ip, #4\n\t"

	// Store the kernel state.
	"SUB	sp, sp, #4\n\t"
	"STR	r2, [ sp, #0 ]\n\t"		// Store the TID
	"STMFD	sp!, { r4-r11 }\n\t"

	// Store information temporarily
	"SUB	sp, sp, #4\n\t"
	"LDR	r3, [ r0, #14*4 ]\n\t"		// The task's SPSR is loaded. 
	"STR	r1, [ r0, #14*4 ]\n\t"		// The next instruction to execute in the user task.	
	"STR	r3, [ sp, #0 ]\n\t"		// The task's SPSR

	// Load the state of the task. 
	"MSR	cpsr_c, #0x1F\n\t"		// Switch to system mode. 
	"MOV	sp, r0\n\t"			
	"LDMFD	sp!, { r0-r12, lr }\n\t"	
	"STR	r0, [ sp, #-4 ]\n\t"		// Temporarily store the r0 (to be able to use this registry). 
	"MSR	cpsr_c, #0x13\n\t"		// Return to supervisor mode. 

	// Pass control to the user task. 
	"LDR	r0, [ sp, #0 ]\n\t"		// Loads the previously stored SPSR.   
	"ADD	sp, sp, #4\n\t"			// "Remove" the top elements of the stack that won't be used anymore. (Kernel's stack). 
	"MSR	cpsr, r0\n\t"  			// Switch to user mode. 
	"LDR	r0, [ sp, #-4 ]\n\t"		// Remove the temporarily stored r0. 
	"ADD	sp, sp, #4\n\t"			// Remove the stored "counter"
	"LDR	PC, [ sp, #-4 ]\n\t"		// Jump to the next instruction in the user task. 

	/*
	// Debugging
	"MOV	r4, r0\n\t"
	"MOV	r0, #1\n\t"
	"MOV	r1, r4\n\t"
	"BL	bwputr\n\t"*/
);

// TODO: Refactor this to a header file.
#define MAX_NUM_ARGUMENTS 10

// TODO: Refactor this functions.
void RetrieveSysCallArgs( int *sysCallArguments, int numArguments, unsigned int taskSP )
{
	//DEBUGGING
	bwprintf( COM2, "RetrieveSysCallArgs: ENTERED");

	// TODO: Add an assertion here. 

	// The arguments are stored in the memory addresses that hold R0-R3 of the user task. 
	// If there are more arguments they are stored in the user task's stack. 	
	int *ptr = ( int * ) taskSP; 
	//ptr += 13; 	// Position it at the R0 address. // TODO: Set this as a constant. 


	/*// DEBUGGING
	int j; 
	for ( j = 0; j < 20; j++ )
	{
		bwprintf( COM2, "DEB VALUES. Stack value: %x\n", *(ptr + j) );
	}*/
	
	int i; 
	for ( i = 0; i < numArguments && i < MAX_NUM_ARGUMENTS ; i++ )
	{
		// Copy the arguments.
		*( sysCallArguments++ ) = *( ptr++ ); 

		// The last register that holds arguments. The next place to look arguments is the normal user task stack. 
		if ( i == 3 ) 
		{
			// TODO: Modify this for more than 4 arguments. 
		}
	}
}

void SetSysCallReturn( int returnValue, unsigned int taskSP )
{
	//DEBUGGING
	bwprintf( COM2, "SetSysCallReturn: ENTERED");

	// The return value is in the address that currently holds R0 for the task. 
	int *ptr = ( int * ) taskSP; 
	//ptr += 13; 	// Position the pointer at the R0 address. // TODO: Set this as a constant. 

	*( ptr ) = returnValue; 
}

void handle_request( int request, Kern_Globals *GLOBALS ) 
{
	//DEBUGGING
	bwprintf( COM2, "handle_request: ENTERED");

	//DEBUGGING
	//bwprintf( COM2, "Handle Request. Request ID: %d Current Task ID: %d \n", request, GLOBALS->schedule.last_active_tid );

	// Create a placeholder for the arguments.
	int sysCallArguments[ MAX_NUM_ARGUMENTS ]; //POINTER???

	Task_descriptor *td = &( GLOBALS->tasks[ GLOBALS->schedule.last_active_tid ]);
	
	//bwprintf( COM2, "Handle Request. First arg: %x  Second arg:  %x \n", sysCallArguments[0], sysCallArguments[1] );
	
	int returnValue;
	unsigned int taskSP =  ( unsigned int ) td->sp;

	switch ( request )
	{
		case CREATE_SYSCALL:
			RetrieveSysCallArgs( sysCallArguments, CREATE_ARGS, taskSP);
			returnValue = sys_create(sysCallArguments[0], (void *) sysCallArguments[1], td, GLOBALS);
			SetSysCallReturn(returnValue, taskSP);
			
			//DEBUGGING
			bwprintf( COM2, "new_tid: %d\n", returnValue);

			break;

		case MYTID_SYSCALL:
			returnValue = sys_mytid(td, GLOBALS);
			SetSysCallReturn(returnValue, taskSP);

			//DEBUGGING
			bwprintf( COM2, "my_tid: %d\n", returnValue);

			break;
		case MYPARENTTID_SYSCALL:
			returnValue = sys_myparenttid(td, GLOBALS);
			SetSysCallReturn(returnValue, taskSP);

			//DEBUGGING
			bwprintf( COM2, "my_tid: %d\n", returnValue);

			break;
		case PASS_SYSCALL:
			sys_pass(td, GLOBALS);

			//DEBUGGING
			bwprintf( COM2, "sys_pass is executed");

			break;
		case EXIT_SYSCALL:
			sys_exit(td, GLOBALS);

			//DEBUGGING
			bwprintf( COM2, "sys_exit is executed");
	
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
	first_td->next_instruction = (unsigned int) first_task_code + ELF_START;
	
	//Updating the queue appropriately////////////////////////
	sched->priority[first_task_priority].td_ptrs[0] = first_td;
	sched->priority[first_task_priority].newest = 0;
	sched->priority[first_task_priority].oldest = 0;
	sched->priority[first_task_priority].size++;

	//Updating the schedule appropriately
	sched->last_issued_tid = 0;
}

// Return:
// tid of the next task to run
int schedule( Kern_Globals * GLOBALS) {
	//DEBUGGING
	bwprintf( COM2, "schedule: ENTERED\n" );

	int p = SCHED_NUM_PRIORITIES - 1;

	// If there are no tasks in all priority queues - PANIC
	while( GLOBALS->schedule.priority[p].size == 0 ) {
		--p;
		if( p < 0 ) panic( "Scheduler has nothing in it's queues." );
	}

	// Get the non-empty queue with the highest priority
	Task_queue *queue = &(GLOBALS->schedule.priority[p]);
	
	//DEBUGGING
	//bwprintf( COM2, "Schedule: Queue: %x\n\r", queue );

	// TD of the task, which should run next
	Task_descriptor *next_td = queue->td_ptrs[queue->oldest];

	//DEBUGGING
	//bwprintf( COM2, "Schedule: TD: %x\n\r", next_td );

	//DEBUGGING
	//bwprintf( COM2, "Schedule: TD->tid: %d\n\r", next_td->tid );

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
int activate( int tid, Kern_Globals *GLOBALS ) {
	//DEBUGGING
	bwprintf( COM2, "activate: ENTERED\n");
	bwprintf( COM2, "activate: TID: %d\n\r", tid );
	
	// Getting TD of the specified task
	Task_descriptor *td = &(GLOBALS->tasks[tid]);

	//DEBUGGING
	//bwprintf( COM2, "activate: TD: %x\n\r", td );
	
	//DEBUGGING
	//bwprintf( COM2, "activate: TD->state: %d\n\r", td->state );

	// ASSERT: the task should be in the READY state
	assert( td->state == READY_TASK, "It should only be possible to activate a READY task" );
	td->state = ACTIVE_TASK;

	//DEBUGGING
	//bwprintf( COM2, "activate: BEFORE ASSEMBLY!!!\n\r" );

	//DEBUGGING
	//bwprintf( COM2, "activate. SINTs. td->sp: %x ; td->next_instruction: %x ; td->lr: %x ; td: %x \n", td->sp, td->next_instruction, td->lr, td );

	unsigned int uisp = (unsigned int) td->sp;			//This is right, judging by memory allocation
	unsigned int uini = (unsigned int) td->next_instruction;	//This is right, judging by the map file and arithmetic :)
	unsigned int uilr = (unsigned int) td->lr;			//This is right, judging by the map file
	unsigned int uitd = (unsigned int) td;				//This seems to be right...

	//DEBUGGING
	//bwprintf( COM2, "activate. UINTs. td->sp: %x ; td->next_instruction: %x ; td->lr: %x ; td: %x \n", uisp, uini, uilr, uitd );

	//Executing using link register
	int request = execute_user_task(uisp, uilr, uitd);

	//DEBUGGING
	bwprintf( COM2, "activate. Request ID: %d\n", request );

	return request;
}

int getNextRequest( Kern_Globals *GLOBALS )
{
	//DEBUGGING
	bwprintf( COM2, "getNextRequest. ENTERED\n");

	return activate( schedule( GLOBALS ), GLOBALS );
}





























