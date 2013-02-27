#include "kernelspace.h"

void timer_hwi_handler( Kern_Globals *GLOBALS ){
	// Retrieve the waiting event from the hwi table
	Task_descriptor *watcher = GLOBALS->scheduler.hwi_watchers[TIMER1_INT_INDEX];
	
	if( watcher != 0 ) {
	
		//Rescheduling the task
		watcher->state = READY_TASK;
		enqueue_tqueue( watcher, &(GLOBALS->scheduler.queues[watcher->priority]) );
		GLOBALS->scheduler.tasks_alive++;

		//Clear the event in hwi events waiting table
		GLOBALS->scheduler.hwi_watchers[TIMER1_INT_INDEX] = 0;
	}
	// Clear source of interrupt. 
	int *timerClrPtr = ( int * )( TIMER1_BASE + CLR_OFFSET );
	*timerClrPtr = 1; // Any value clears the src of the interrupt. 
	
	int *vicPtr = ( int * )( INT_CONTROL_BASE_1 + INT_VEC_ADDRESS_OFFSET );
	*vicPtr = 0; 
}

void uart1_hwi_handler( Kern_Globals *GLOBALS ) {
	int *uart1_common_interrupt = ( int * )( UART1_BASE + UART_INTR_OFFSET ); 
	int uart1_interrupt = *uart1_common_interrupt;
	
	Task_descriptor *waiting_task = 0;
	Task_descriptor *waiting_task_init = 0; 
	
	debug( DBG_KERN, "UART1_HWI_HANDLER: interrupt recieved [%d]",
			*uart1_common_interrupt );

	// Is there data to be received?
	if ( uart1_interrupt & UART_RX_INT_STATUS ) {
		
		// Retrieve the waiting event from the hwi table. 
		waiting_task = ( Task_descriptor * ) GLOBALS->scheduler.hwi_watchers[UART1_RECEIVE_READY];
		GLOBALS->scheduler.hwi_watchers[UART1_RECEIVE_READY] = 0;
		
		// Read the character -> This also clears the interrupt. 
		int c = *( ( int * ) ( UART1_BASE + UART_DATA_OFFSET ) ); 
		
		if( waiting_task != 0 ) {
			int *buffer = ( int * ) waiting_task->event_char; 
			*buffer = c;
		}
	} 
	else if ( 
			uart1_interrupt & UART_TX_INT_STATUS || 
			uart1_interrupt & UART_MODEM_INT_STATUS ) {
		
		// Check if there ara tasks waiting for the events. 
		waiting_task = ( Task_descriptor * ) GLOBALS->scheduler.hwi_watchers[UART1_SEND_READY];
		GLOBALS->scheduler.hwi_watchers[UART1_SEND_READY] = 0;
		
		waiting_task_init = ( Task_descriptor * ) GLOBALS->scheduler.hwi_watchers[UART1_INIT_SEND];
		GLOBALS->scheduler.hwi_watchers[UART1_INIT_SEND] = 0;		
		
		// Clear the interrupt sources. 
		if ( uart1_interrupt & UART_TX_INT_STATUS ) {
			// This interrupt can't be cleared until something is written to the FIFO. 
			// However, if some trash is put there just to clear the interrupt, that 
			// will be sent to the train, producing unexpected results. Therefore, the 
			// interrupt is not cleaned, but DISABLED. Next time it's needed it will  
			// be re-enabled (during AwaitEvent). 
			int *uart1_ctrl, temp; 
			uart1_ctrl = ( int * ) ( UART1_BASE + UART_CTLR_OFFSET ); 
			temp = *uart1_ctrl; 
			*uart1_ctrl = temp & ~TIEN_MASK; 
		}
		
		if ( uart1_interrupt & UART_MODEM_INT_STATUS ) {
			// The interrupt is cleared. 
			*uart1_common_interrupt = 0; 
			
			// The interrupt is also disabled, as what happened with UART_TX_INT_STATUS. 
			int *uart1_ctrl, temp; 
			uart1_ctrl = ( int * ) ( UART1_BASE + UART_CTLR_OFFSET ); 
			temp = *uart1_ctrl; 
			//*uart1_ctrl = temp & ~MSIEN_MASK;
		}
		
	} else {
		// This was an unexpected interrupt; we don't care about it. 
	}
	
	// If there were tasks waiting for these events, reschedule them. 
	if( waiting_task != 0 ) {	
		//Rescheduling the task
		waiting_task->state = READY_TASK;
		enqueue_tqueue( waiting_task, &(GLOBALS->scheduler.queues[waiting_task->priority]) );
		GLOBALS->scheduler.tasks_alive++;
	}
	
	if ( waiting_task_init != 0 ) {
		//Rescheduling the task
		waiting_task_init->state = READY_TASK;
		enqueue_tqueue( waiting_task_init, &(GLOBALS->scheduler.queues[waiting_task_init->priority]) );		
		GLOBALS->scheduler.tasks_alive++;
	}
	
	// Clear the interrupt in the ICU.
	int *vicPtr = (int *)( INT_CONTROL_BASE_2 + INT_VEC_ADDRESS_OFFSET );
	*vicPtr = 0;
}

void uart2_hwi_handler( Kern_Globals *GLOBALS ){

	int *uart2_common_interrupt = ( int * )( UART2_BASE + UART_INTR_OFFSET ); 
	Task_descriptor *waiting_task = 0;
	int c;

	debug( DBG_KERN, "UART2_HWI_HANDLER: interrupt recieved [%d]",
			*uart2_common_interrupt );
	
	// Is there data to be received?
	int temp = *uart2_common_interrupt; 
	if( temp & UART_RX_INT_STATUS ) { 
		
		// Retrieve the waiting event from the hwi table. 
		waiting_task = ( Task_descriptor * ) GLOBALS->scheduler.hwi_watchers[UART2_RECEIVE_READY];
		GLOBALS->scheduler.hwi_watchers[UART2_RECEIVE_READY] = 0;
		
		// Read the character -> This also clears the interrupt.
		c = *(( int * )( UART2_BASE + UART_DATA_OFFSET ) );
		
		if( waiting_task != 0 ) {
			todo_debug( waiting_task->event_char, 2 );
			int *buffer = ( int * ) waiting_task->event_char; 
			*buffer = c;
		}
	} 
	else if ( temp & UART_TX_INT_STATUS ) {
		// Retrieve the waiting event from the hwi table.
		waiting_task = ( Task_descriptor * ) GLOBALS->scheduler.hwi_watchers[UART2_SEND_READY];
		GLOBALS->scheduler.hwi_watchers[UART2_SEND_READY] = 0;
		
		// Deactivate the interrupt.
		// NOTE: This interrupt also needs to be DISABLED. 
		int *uart2_ctrl, temp; 
		uart2_ctrl = ( int * ) (  UART2_BASE + UART_CTLR_OFFSET ); 
		temp = *uart2_ctrl; 
		*uart2_ctrl = temp & ~TIEN_MASK;
	}
	else {
		// This was an unexpected interrupt; just ignore it. 
	}
	
	// Clear the interrupt in the ICU.
	// If there was a task waiting for these events, reschedule it. 
	if( waiting_task != 0 ) {
		//Rescheduling the task
		waiting_task->state = READY_TASK;
		enqueue_tqueue( waiting_task, &(GLOBALS->scheduler.queues[waiting_task->priority]) );
		GLOBALS->scheduler.tasks_alive++;
		//todo_debug( c, 1 );
	}
	
	// Clear the interrupt in the ICU.
	int *vicPtr = (int *)( INT_CONTROL_BASE_2 + INT_VEC_ADDRESS_OFFSET );
	*vicPtr = 0;
}
