#include "kernelspace.h"

int main( ) {
  
	asm( 
		".EQU	IRQ_MODE,			0x92"			"\n\t"
		".EQU	SVC_MODE,			0x93"			"\n\t"
	
		// Show SVC sp
		//"MOV	r1, #20"							"\n\t"
		//"MRS		r0, cpsr"							"\n\t"
		//"MOV	r0, sp"							"\n\t"
		//"BL		todo_debug"							"\n\t"
	
		// Move to IRQ
		"MSR	cpsr_c, #IRQ_MODE"						"\n\t"
		"MOV	r1, #20"							"\n\t"
		"MOV	r0, sp"							"\n\t"
		"BL		todo_debug"							"\n\t"
	
		// Return to SVC
		"MSR	cpsr_c, #SVC_MODE"						"\n\t"
	); 
	
	Kern_Globals KERN_GLOBALS;		// "Global" kernel structure
	initialize( &KERN_GLOBALS );	// includes starting the first user task

	int request;

	FOREVER {
		request = getNextRequest( &KERN_GLOBALS );
		// debug( DBG_SYS, "KERNEL: handling request [tasks alive: %d]",
		// 	KERN_GLOBALS.scheduler.tasks_alive );
		// if( KERN_GLOBALS.scheduler.tasks_exited > 0 &&
		// 	KERN_GLOBALS.scheduler.tasks_alive < 4 )
		// 	return 0;
		
		if( KERN_GLOBALS.scheduler.tasks_exited > 10 ) return 0;

		handle_request( request, &KERN_GLOBALS );
	}

	return 0;
}
