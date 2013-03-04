#include "kernelspace.h"

int main( ) {
	Kern_Globals KERN_GLOBALS;		// "Global" kernel structure
	initialize( &KERN_GLOBALS );	// includes starting the first user task

	int request;
	int exits = 0;

	FOREVER {
		request = getNextRequest( &KERN_GLOBALS );
		if( request == EXIT_SYSCALL ) if( exits++ > 1 ) return 666;
		handle_request( request, &KERN_GLOBALS );
	}
}
