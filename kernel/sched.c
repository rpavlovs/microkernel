#include "kernel/sched.h"
#include "kernel/kernel_globals.h"
#include "lib/bwio.h"
#include "kernel/helpers.h"
#include "config/mem_init.h"

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

extern void execute_user_task(unsigned int userTaskSP, unsigned int nextInstruction, unsigned int taskID);

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

// Add task to scheduler as ready to run
int add_task( int priority, void (*code) ( ), Kern_Globals *GLOBALS ) {
	
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
	
	//DEBUGGING
	bwprintf( COM2, "Schedule: Queue: %x\n\r", queue );

	// TD of the task, which should run next
	Task_descriptor *next_td = queue->td_ptrs[queue->oldest];

	//DEBUGGING
	bwprintf( COM2, "Schedule: TD: %x\n\r", next_td );

	//DEBUGGING
	bwprintf( COM2, "Schedule: TD->tid: %d\n\r", next_td->tid );

	if( ++(queue->oldest) >= SCHED_QUEUE_LENGTH ) queue->oldest = 0;
	--(queue->size);

	return next_td->tid;
}

// Start running the task with specified tid
// Return:
// interrupt ID of the first recieved interrupt
int activate( int tid, Kern_Globals *GLOBALS ) {
	//DEBUGGING
	bwprintf( COM2, "activate: TID: %d\n\r", tid );
	
	// Getting TD of the specified task
	Task_descriptor *td = &(GLOBALS->tasks[tid]);

	//DEBUGGING
	bwprintf( COM2, "activate: TD: %x\n\r", td );
	
	//DEBUGGING
	bwprintf( COM2, "activate: TD->state: %d\n\r", td->state );

	// ASSERT: the task should be in the READY state
	assert( td->state == READY_TASK, "It should only be possible to activate a READY task" );
	td->state = ACTIVE_TASK;

	//DEBUGGING
	bwprintf( COM2, "activate: BEFORE ASSEMBLY!!!\n\r" );

	//DEBUGGING
	bwprintf( COM2, "activate. SINTs. td->sp: %x ; td->next_instruction: %x ; td->lr: %x ; td: %x \n", td->sp, td->next_instruction, td->lr, td );

	unsigned int uisp = (unsigned int) 0x01d00000;  // td->sp;	//This is right, judging by memory allocation
	unsigned int uini = (unsigned int) td->next_instruction;	//This is right, judging by the map file and arithmetic :)
	unsigned int uilr = (unsigned int) td->lr;			//This is right, judging by the map file
	unsigned int uitd = (unsigned int) td;				//This seems to be right...

	//DEBUGGING
	bwprintf( COM2, "activate. UINTs. td->sp: %x ; td->next_instruction: %x ; td->lr: %x ; td: %x \n", uisp, uini, uilr, uitd );

	//Executing using link register
	execute_user_task(uisp, uini, uitd);

	//Executing using next instruction
	//execute_user_task(uisp, uini, uitd);


///////////////////////////////////////////////////////////////////////////////////////////////////
/*	// Save kernel registers
	asm (	"mov	ip, sp"																								"\n\t" );
	asm (	"stmfd	sp!, {r4, r5, r6, r7, r8, r9, r10, fp, ip, lr, pc}"	"\n\t" );
	
	// Set PSR of the task to be activated
	asm ( "msr	spsr, %0" 																							"\n\t"
				:: "r" (td->spsr) );

	// Set LR to jump to the next active process
	asm ( "mov	lr, %0" 																								"\n\t"
				:: "r" (td->lr) );

	// Switch to system mode
	asm ( "msr	cpsr, #0xdf" 																					"\n\t" );
	
	// Set stack pointer of the active process
	asm ( "mov 	sp, %0" 																							"\n\t"
				:: "r" (td->sp) );

	// Load all process registers from *it's* stack
	asm ( "ldmfd sp!, {r0-r12, lr}" 																	"\n\t");

	// go back to supervisor mode
	asm ( "msr cpsr, #0xd3"																						"\n\t");
	
	// Jump to start executing active task
	// NOTE:this should set cpsr after the jump to contents of spsr 
	asm ( "movs pc, lr"																								"\n\t");*/
///////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: return the interrupt ID
	return 0;
}

int getNextRequest( Kern_Globals *GLOBALS ) 
{
	return activate( schedule( GLOBALS ), GLOBALS );
}
