#include "kernelspace.h"

void init_message_queues( Kern_Globals *GLOBALS ) {
	int tid ;
	for( tid = 0; tid < MAX_NUM_TASKS; ++tid) {
		Message_queue *mailbox = &(GLOBALS->tasks[tid].mailbox);
		mailbox->newest = -1;
		mailbox->oldest = 0;
		mailbox->size = 0;
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

void init_io() {
	int temp; 
	int *uart_ctrl, *mdm_ctrl, *fifo, *low, *mid; 
	
	// -- UART 1 -> Interacts with the train. ---------------------------------
	uart_ctrl = (int *)( UART1_BASE + UART_CTLR_OFFSET );
	
	// -> Enable interrupts. 
	// 1. Receive interrupt		(UARTRXINTR  - RIEN_MASK )
	// 2. Transmit interrupt	(UARTTRXINTR - TIEN_MASK )
	// 3. Modem Status			(UARTMSINTR  - MSIEN_MASK)
	// NOTE: Both, the transmit interrupt and modem status are enabled during
	// await event. That's the reason why they are not enabled here. 
	temp = *uart_ctrl; 
	*uart_ctrl = temp | RIEN_MASK;
	
	// -> Set the UART speed (baud rate) -> 2400 bps. 
	low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
	mid = (int *)( UART1_BASE + UART_LCRM_OFFSET );
	*low = 0xbf;
	*mid = 0x0;
	
	// -> Configure UART (Disable FIFOs, and enable two step bits frame) 
	fifo = (int *)( UART1_BASE + UART_LCRH_OFFSET );
	temp = *fifo | STP2_MASK;	// TODO: Check if the two step bits frame is really needed. 
	*fifo = temp & ~FEN_MASK;
	
	// -> Enable modem (to check for CTS).
	mdm_ctrl = (int *)( UART1_BASE + UART_MDMCTL_OFFSET);
	*mdm_ctrl = (*mdm_ctrl | 1);
	
	// -- UART 2 -> Interacts with the console. -------------------------------
	uart_ctrl = (int *)( UART2_BASE + UART_CTLR_OFFSET );
	
	// -> Enable interrupts. 
	// 1. Receive interrupt		(UARTRXINTR  - RIEN_MASK )
	// 2. Transmit interrupt	(UARTTRXINTR - TIEN_MASK )
	// NOTE: As with UART1, the Transmit interrupt is enabled during await event.
	//*uart_ctrl = INT_RESET_VALUE | RIEN_MASK;
	temp = *uart_ctrl; 
	*uart_ctrl = temp | RIEN_MASK | RTIEN_MASK;	
	
	// -> Set the UART speed (baud rate) -> 115200 bps.
	low = (int *)( UART2_BASE + UART_LCRL_OFFSET );
	mid = (int *)( UART2_BASE + UART_LCRM_OFFSET );
	*mid = 0x0;
	*low = 0x3;
	
	// -> Configure UART (Disable FIFOs)
	fifo = (int *)( UART2_BASE + UART_LCRH_OFFSET );
	temp = *fifo; 
	*fifo = temp & ~FEN_MASK;
	
	// NOTE: The actual interrupts (in the ICU) are enabled during 
	// context switch initialization. 
}

void init_hardware() {
	
	// Start Debug timer (Timer 4)
    int *hi = (int *)Timer4ValueHigh;
    *hi = (1 << 8);
	
	// TODO: Re-enable this when we stop using busy wait. 
	init_io(); 
}

void initialize( Kern_Globals *GLOBALS ) {

	initialize_context_switching(); 
	
	init_hardware();

    //initialize_context_switching(); 

	init_message_queues( GLOBALS );

	init_task_descriptors( GLOBALS );

	init_schedule( 15, first_task, GLOBALS );
}
