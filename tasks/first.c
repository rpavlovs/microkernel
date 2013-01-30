#include "kernel/syscall.h"
#include "tasks/first.h"
#include "tasks/nameserver.h"
#include "lib/bwio.h"
#include "config/ts7200.h"

void sub() {
	int status;

	bwprintf( COM2, "Entering sub\n" );

	status = RegisterAs("the_sub");
	bwprintf( COM2, "Sub registered with status: %d\n", status);

	bwprintf( COM2, "WhoIs the_sub: %d\n", WhoIs("the_sub") );
	bwprintf( COM2, "WhoIs the_first: %d\n", WhoIs("the_first") );

	bwprintf( COM2, "Exiting sub\n" );
	Exit();
}



void first_task() {
	int tid, status;

	tid = Create( 14, nameserver );
	bwprintf( COM2, "Nameserver true TID: %d\n", tid);

	status = RegisterAs("the_first");
	bwprintf( COM2, "First registered with status: %d\n", status);

	tid = Create( 4, sub );
	bwprintf( COM2, "Sub() is created. TID: %d\n", tid );

	Pass();

	bwprintf( COM2, "WhoIs the_sub: %d\n", WhoIs("the_sub") );

	bwprintf( COM2, "Exiting first\n" );
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

// first user task gets launched with priority 8
void a1_first_task() {

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
