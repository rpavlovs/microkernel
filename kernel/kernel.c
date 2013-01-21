#include "config/ts7200.h"
#include "kernel/sched.h"
#include "kernel/kernel_globals.h"

#define FOREVER for( ; ; )

void initialize(){
	//Where is the kernel entry?
	//Turn off interrupts in ICU!

	//Prepare the kernel data structures
		//Where is the kernel stack pointer right now? (Kernel stack pointer is in the end of the RAM memory)
		//What does the stack look like? (Stack goes downwards from the end of the RAM memory)
		//Do we want it there? YES
		//Do we want it somewhere else? NO
		//This is the last chance to change it! OK :)
		//If we are going to change it - it is a good idea to keep changed stuff around! WHATEVER :)

	//Kernel data structures
		//An array of empty ready queues
		//A pointer to the TD of the active task
		//An array of TDs
		//A free list of pointers to free TDs

	//Prepare memory to be used by tasks - DONE
		//Allocate task memory - DONE

	//Create first user task
		//Can run with interrupts turned off by now
	//NOTE: the place where the kernel starts executing has the global name main which cannot be reused

//Where is the kernel stack pointer right now?
__asm__(
	"mov r0, #1\n"
	"mov r1, sp\n"
	"bl bwputr\n"
);

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
