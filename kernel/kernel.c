#include "config/ts7200.h"
#include "kernel/sched.h"
#include "kernel/syscall.h"
#include "kernel/kernel_globals.h"
#include "kernel/init.h"
#include "lib/bwio.h"

#define FOREVER for( ; ; )

int main( ) {
  
	Kern_Globals KERN_GLOBALS;	// "Global" kernel structure
	initialize( &KERN_GLOBALS );	// includes starting the first user task

	int request;

	FOREVER {
		request = getNextRequest( &KERN_GLOBALS );
		if( request == EXIT_SYSCALL && KERN_GLOBALS.schedule.tasks_alive == 1)
			return 0;
		handle_request( request, &KERN_GLOBALS );
		// bwprintf( COM2, "%c", bwgetc(COM2));
	}

	return 0;
}
