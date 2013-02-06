#include "userspace.h"

void a1() {
	int tid;
	
	tid = Create( 5, other_task);
	bwprintf( COM2, "Created: %d.\n\r", tid);
	
	tid = Create( 5, other_task);
	bwprintf( COM2, "Created: %d.\n\r", tid);

	tid = Create( 12, other_task);
	bwprintf( COM2, "Created: %d.\n\r", tid);

	tid = Create( 15, other_task);
	bwprintf( COM2, "Created: %d.\n\r", tid);

	bwprintf( COM2, "First: exiting.\n\r");
	Exit();
}

void other_task() {
	int mytid = MyTid();
	int myparenttid = MyParentTid();
	bwprintf( COM2, "My tid: %d\tMy parent tid:%d\n\r", mytid, myparenttid);
	Pass();
	bwprintf( COM2, "My tid: %d\tMy parent tid:%d\n\r", mytid, myparenttid);
	Exit();
}
