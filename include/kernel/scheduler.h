#ifndef ___SCHED___
#define ___SCHED___

#include "kernelspace.h"


// Description:
// Init schedule structure in the kernel globals.
// Specify the first task to be executed and it's priority 

void init_schedule( int, void (*) ( ), Kern_Globals * );

// Description:
// Find an unused task descriptor
// Set it up for a new task
// Add it to the specific scheduler queue 
//
// Return:
// task id of the new process
// -1 if wrong task priority was given
// -2 if no task descriptors left

int add_task( int, void (*) ( ), Kern_Globals * );

// Description:
// Run the next scheduled task until interrupt recieved
// 
// Return:
// Interrupt id 

int getNextRequest( Kern_Globals * );

// TODO: Description
//       

void handle_request( int, Kern_Globals * );

#endif
