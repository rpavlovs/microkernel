#include "kernelspace.h"

int main( ) {
  
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
