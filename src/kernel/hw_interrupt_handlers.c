#include "kernelspace.h"

void timer_hwi_handler( Kern_Globals *GLOBALS ){
	// Retrieve the waiting event from the hwi table
	Task_descriptor *watcher = GLOBALS->scheduler.hwi_watchers[TIMER1_INT_INDEX];
	
	if( watcher != 0 ) {
	
		//Rescheduling the task
		watcher->state = READY_TASK;
		enqueue_tqueue( watcher, &(GLOBALS->scheduler.queues[watcher->priority]) );
		
		//Clear the event in hwi events waiting table
		GLOBALS->scheduler.hwi_watchers[TIMER1_INT_INDEX] = 0;
	}
	// Clear source of interrupt. 
	int *timerClrPtr = (int *)( TIMER1_BASE + CLR_OFFSET );
	*timerClrPtr = 1; // Any value clears the src of the interrupt. 
	
	int *vicPtr = (int *)( INT_CONTROL_BASE_1 + INT_VEC_ADDRESS_OFFSET );
	*vicPtr = 0; 
}
