#include "kernel/kernel_globals.h"
#include "kernel/sysfuncs.h"
#include "lib/bwio.h"

int sys_create( int priority, void (*code) ( ), Kern_Globals *GLOBALS ) {
	
	return 0;
}

int sys_mytid(Task_descriptor *td, Kern_Globals *GLOBALS )
{
	return td->tid;
}

int sys_myparenttid(Task_descriptor *td, Kern_Globals *GLOBALS )
{
	return td->parent_tid;
}

void sys_pass(Task_descriptor *td, Kern_Globals *GLOBALS ) {
	//If priority queue size == 1 => do nothing
	//Else => dequeue the task, enqueue the task at the end
}

void sys_exit(Task_descriptor *td, Kern_Globals *GLOBALS ) {
	//Delete the task from the priority queue
}
