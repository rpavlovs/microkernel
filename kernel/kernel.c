#include "config/ts7200.h"
#include "kernel/sched.h"
#include "kernel/kernel_globals.h"
#include "kernel/init.h"

#define FOREVER for( ; ; )

void handle_request( int request ) {

}

int main( ) {
  
	Kern_Globals KERN_GLOBALS;	// "Global" kernel structure
	initialize( &KERN_GLOBALS );	// includes starting the first user task

	int request;
  
	FOREVER {
		request = getNextRequest( &KERN_GLOBALS );
		//handle_request( request );
	}

	return 0;
}
