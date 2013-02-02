#include "kernel/context_switch.h"
#include "kernel/kernel_globals.h"
#include "kernel/syscall.h"
#include "kernel/sysfuncs.h"


void swi_main_handler() {

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
}

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

void execute_user_task() {
	
	asm (
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
}

void RetrieveSysCallArgs( int *sysCallArguments, int numArguments, unsigned int taskSP )
{
	// TODO: Add an assertion here.

	// The arguments are stored in the memory addresses that hold R0-R3 of the user task.
	// If there are more arguments they are stored in the user task's stack. 	
	int *ptr = ( int * ) taskSP; 

	/*// DEBUGGING
	int j; 
	for ( j = -1; j < 30; j++ )
	{
	 	bwprintf( COM2, "DEB VALUES. Index:%d Stack value: %d\n", j, *(ptr + j) );
	}*/
	
	int i; 
	for ( i = 0; i < numArguments && i < MAX_NUM_ARGUMENTS ; i++ )
	{
		// Copy the arguments.
		*( sysCallArguments++ ) = *( ptr++ );

		// The last register that holds arguments. 
		//The next place to look arguments is the normal user task stack.
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
	unsigned int taskSP = ( unsigned int ) td->sp;

	switch ( request )
	{
		case CREATE_SYSCALL:
			RetrieveSysCallArgs( sysCallArguments, CREATE_ARGS, taskSP);
			
			returnValue = sys_create(
								sysCallArguments[0],
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
			returnValue = sys_testcall(
							sysCallArguments[0], 
							sysCallArguments[1], 
							sysCallArguments[2], 
							sysCallArguments[3], 
							sysCallArguments[4], 
							sysCallArguments[5]);
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

