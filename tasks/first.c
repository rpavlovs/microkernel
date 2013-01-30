#include "kernel/syscall.h"
#include "tasks/first.h"
#include "lib/bwio.h"
#include "config/ts7200.h"

void other_task() {
	int mytid = MyTid();
	int myparenttid = MyParentTid();
	bwprintf( COM2, "My tid: %d\tMy parent tid:%d\n\r", mytid, myparenttid);
	Pass();
	bwprintf( COM2, "My tid: %d\tMy parent tid:%d\n\r", mytid, myparenttid);	
	Exit();
}

// first user task gets launched with priority 8
void first_task() {

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

void test_task(){

	int sum;

	sum = TestCall(11,22,33,44,55,66);
	bwprintf( COM2, "The sum is:%d", sum);

	Exit();
}
