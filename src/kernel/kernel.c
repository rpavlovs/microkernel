#include "kernelspace.h"

int main( ) {
  
	Kern_Globals KERN_GLOBALS;		// "Global" kernel structure
	initialize( &KERN_GLOBALS );	// includes starting the first user task

	int request;

	bwprintf( COM2, "Blabla\n\r" );
	
	FOREVER {
		request = getNextRequest( &KERN_GLOBALS );
		
		bwprintf( COM2, "Not again:) Request: %d \n\r", request );
		if( request == EXIT_SYSCALL && KERN_GLOBALS.schedule.tasks_alive <= 3 )
			return 0;
		handle_request( request, &KERN_GLOBALS );
		// bwprintf( COM2, "%c", bwgetc(COM2));
	}

	return 0;
}
