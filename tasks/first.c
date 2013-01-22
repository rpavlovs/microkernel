#include "kernel/syscall.h"

// first user task gets launched with priority 8
void first_task() {
	bwprintf( COM2, "First: Howdy, user!\n\r");
}

// first user task gets launched with priority 8
void first_real_task() {
	int tid;
	
	tid = Create( 1, other_task);
	bwprintf( COM2, "Created: %d.", tid);
	
	tid = Create( 5, other_task);
	bwprintf( COM2, "Created: %d.", tid);

	tid = Create( 12, other_task);
	bwprintf( COM2, "Created: %d.", tid);

	tid = Create( 15, other_task);
	bwprintf( COM2, "Created: %d.", tid);

	bwprintf( COM2, "First: exiting.");
	Exit();
}

void other_task() {
	bwprintf( COM2, "My tid: %d\tMy parent tid:%d\n\r", MyTid(), MyParentTid());
	Pass();
	bwprintf( COM2, "My tid: %d\tMy parent tid:%d\n\r", MyTid(), MyParentTid());	
	Exit();
}