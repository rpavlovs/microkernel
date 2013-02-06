#include "kernelspace.h"

void timer_hwi_handler( Kern_Globals *GLOBALS ){
	// Clear source of interrupt. 
	int *timerClrPtr = (int *)( TIMER1_BASE + CLR_OFFSET );
	*timerClrPtr = 1; // Any value clears the src of the interrupt. 
	
	// TODO: Here it's required to communicate with the clock server. 
}