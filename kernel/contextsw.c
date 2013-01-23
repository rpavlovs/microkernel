/*
	First Context Switch Test
*/
#include <bwio.h>
#include <ts7200.h>


// -------------------------------------------------------------------------------
// Assembly Code
// -------------------------------------------------------------------------------
__asm__(
	
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
	"\nswi_main_handler:\n\t"

	// Save in the stack the arguments (in the registers) since they might get erased. 
	// NOTE: The current SP is the SP_SVC, so the user SP is not affected. 
	"MSR	CPSR_c, #0x1F\n\t"		// Switch to system mode. 
	"SUB	sp, sp, #4\n\t"			// Leave space for SPSR. 
	"STMFD	sp!, { r0-r12, lr }\n\r"	// Store all the registers (except 13-sp- and 15-pc-).
	"MOV	r0, sp\n\r"			// Store the task's SP so that it can be stored later into the TR.
	"MSR	cpsr_c, #0x13\n\t"		// Return to supervisor mode. 

	// Get the value of the system call
	"MRS	r1, spsr\n\t"			// Store in the task's stack the spsr (the original CPSR of the task). 
	"STR	r1, [ r0, #14*4 ]\n\t"
	"MOV	r1, lr\n\t"			// Store the return value in the r1 so that it is stored later into the TR. 

	// Restore the kernel state
	"LDMFD	sp!, { r4-r11 }\n\t"

	// Execute the C system call handler
	"LDR	r2, [ lr, #-4 ]\n\t"		// Store the next instruction to run in the r1 so that it is stored later into the TR. 
	"BIC	r2, r2, #0xff000000\n\t"	
	"LDR	r3, [ sp, #0 ]\n\t"		// The pointer to the active TD is loaded into register 3.
	"BL	ExecuteCSWIHandler\n\t"
	
	// Return control to the kernel C code. 
	"sub	sp, fp, #12\n\t"
	"LDMFD	sp, { fp, sp, pc }\n\t"

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


); 

// -------------------------------------------------------------------------------
// Constants (will be in a header file)
// -------------------------------------------------------------------------------
#define SWI_VECTOR_ADDR 0x8
#define FIRST_ADDRESS 0x00218000
#define TEST_SP 0x01d00000
#define TEST_LR 0x01c00000
#define INITIAL_ADDR 

// -------------------------------------------------------------------------------
// Structure Declaration (will be in a header file)
// -------------------------------------------------------------------------------
typedef struct{
	int tid; 
	unsigned int taskSP; 
	unsigned int nextInstruction; 
} Task;

// -------------------------------------------------------------------------------
// Function Declarations (will be in a header file)
// ------------------------------------------------------------------------------- 
extern void swi_main_handler(); 

extern void execute_user_task( unsigned int userTaskSP, unsigned int nextInstruction, unsigned int taskID ); 

//__swi(0) int multiply_two(int, int);


// -------------------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------------------
void ExecuteCSWIHandler( unsigned int taskSP, unsigned int nextAddress, unsigned int swiValue, unsigned int activeTID )
{
	bwprintf( COM2, "kerxit.c: Updating Active Task   TaskSP: %d    ReturnValue: %d    SWIValue: %d    activeTID: %d .\n\r", taskSP, nextAddress, swiValue, activeTID );
	
	// Update the task. 
	Task *task = ( Task * ) activeTID; 
	task->taskSP = taskSP; 
	task->nextInstruction = nextAddress; 
}

// -------------------------------------------------------------------------------
// Initialization
// -------------------------------------------------------------------------------
void userTask()
{
	while( 1 )
	{
		bwprintf( COM2, "kerxit.c: User Task Starts.\n\r" );
	
		// ... Work is done. 
		__asm__(
			"\n\tswi	10\n\t"
		); 

		bwprintf( COM2, "kerxit.c: User Task Ends.\n\r" );
	}
}

void initialize( Task *task )
{
	// Turn OFF FIFO to enable BW. 
	bwsetfifo( COM2, OFF );

	// Install the SWI Handler that will deal with all the Software Interrupts. 
	unsigned int *swi_vector = ( unsigned int * ) SWI_VECTOR_ADDR;
	installSwiHandler( ( unsigned int ) swi_main_handler + FIRST_ADDRESS , swi_vector ); 

	// Create the Initial Task ID. 
	//Task task;
	task->tid = 1;
	task->taskSP =  TEST_SP; 
	task->nextInstruction = ( ( unsigned int ) userTask) + FIRST_ADDRESS; 

	// Initialize the task (This will be done using Create.)
	int *ptr = ( int * ) task->taskSP; 
	*( ptr + 14 ) = 0x10;	// SPSR 
	
	int i = 0; 
	for ( i = 0; i < 10; i++ )
	{
		*( ptr++ ) =	i;  	// R0-R9
	}
	
	// SL - 10
	*( ptr++ ) = task->taskSP + (15 * 4); // FP - 11
	*( ptr++ ) = 11;		// IP - 12
	*( ptr++ ) = TEST_LR; // LR	
}

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

void kernel_enter( Task *activeTask )
{
	//unsigned int userTaskSP, unsigned int nextInstruction

	int i = 0; 
	for ( i = 0; i < 4; i++ )
	{
		unsigned int sp = ( unsigned int ) activeTask->taskSP;
		unsigned int nextInstruction = ( unsigned int ) activeTask->nextInstruction; 
		unsigned int taskId = ( unsigned int ) activeTask; 

		execute_user_task( sp, nextInstruction, taskId ); 
	}
	
	/*
	__asm__(		
		"MOV	r0, #0x1d000000\n\t"
		"LDR	r1, userTask\n\t"
		"BL	execute_user_task\n\t"
	);*/
	//unsigned int sp = 0x1d000000;
	//unsigned int 
	//execute_user_task(  ); 
	bwprintf( COM2, "kerxit.c: END HAHAHA!!!!!!!!!!!!!!!.\n\r" );
}

int main( int argc, char *argv[] )
{
	// Declare kernel data structures. 
	Task task;
	initialize( &task ); 
	
	//for ( i = 0; i < 4; i++ )
	//{
		//kernel_exit(); 
		kernel_enter( &task ); 
	//}

	return 0; 
}

