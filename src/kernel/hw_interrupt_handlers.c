#include "kernelspace.h"

void timer_hwi_handler( Kern_Globals *GLOBALS ){
	// Retrieve the waiting event from the hwi table
	Task_descriptor *td_pointer = GLOBALS->schedule.hwi_events_waiting_table[TIMER1_INT_INDEX];
	
	if( td_pointer != 0 ) {
	
		//Rescheduling the task
		Task_descriptor *td = (Task_descriptor *) td_pointer;
		Task_queue *pqueue = &(GLOBALS->schedule.priority[td->priority]);
		td->state = READY_TASK;
		enqueue_tqueue(td, pqueue);
		
		//Clear the event in hwi events waiting table
		GLOBALS->schedule.hwi_events_waiting_table[TIMER1_INT_INDEX] = 0;
	}
	// Clear source of interrupt. 
	int *timerClrPtr = (int *)( TIMER1_BASE + CLR_OFFSET );
	*timerClrPtr = 1; // Any value clears the src of the interrupt. 
	
	int *vicPtr = (int *)( INT_CONTROL_BASE_1 + INT_VEC_ADDRESS_OFFSET );
	*vicPtr = 0; 
}
