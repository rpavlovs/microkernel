#include "config/ts7200.h"
#include "kernel/sched.h"
#include "kernel/kernel_globals.h"

#define FOREVER for( ; ; )

void initialize( Kern_Globals *KERN_GLOBALS ) {

	// init_schedule( &( KERN_GLOBALS->schedule ) );

}

void handle_request( int request ) {

}

int main( ) {
  
	Kern_Globals KERN_GLOBALS;
  initialize( &KERN_GLOBALS );  // includes starting the first user task
  
  int request;
  
  FOREVER {
    request = getNextRequest( &(KERN_GLOBALS.schedule) );
    handle_request( request );
  }

  return 0;
}
