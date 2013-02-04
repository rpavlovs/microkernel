#include "kernelspace.h"

int main( ) {
  
	Kern_Globals KERN_GLOBALS;		// "Global" kernel structure
	initialize( &KERN_GLOBALS );	// includes starting the first user task

	Interrupt_info request;

	FOREVER {
		getNextRequest( &request, &KERN_GLOBALS );
		if( request.type == EXIT_SYSCALL && KERN_GLOBALS.schedule.tasks_alive <= 3 )
			return 0;
		handle_request( &request, &KERN_GLOBALS );
	}

	return 0;
}
