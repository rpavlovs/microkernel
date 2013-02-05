#include "userspace.h"


void sub() {
	int sender_tid;
	char buf64[64];
	char buf4[4];

	Receive( &sender_tid, buf4, 4 );
	Reply( sender_tid, buf4, 4 );

	Receive( &sender_tid, buf64, 64 );
	Reply( sender_tid, buf64, 64 );

	Exit();
}

void first_task() {
	debug( DBG_CURR_LVL, DBG_SYS, "> FIRST_TASK: start with tid %d", MyTid() );
	
	int tid;
	tid = Create( 14, nameserver );
	assert( tid == 1, "Nameserver should have task id of 1" );

	tid = Create( 14, timeserver );

	bwprintf( COM2, "It's %d ticks since timeserver was initialized.\n", Time() );

	bwprintf( COM2, "It's %d ticks since timeserver was initialized.\n", Delay( 10 ) );

	debug( DBG_CURR_LVL, DBG_SYS, "> FIRST_TASK: exit" );
	Exit();
}


void first_task_aaa() {
	bwprintf( COM2, "First is entered\n" );
	int tid, status;

	tid = Create( 14, nameserver );
	bwprintf( COM2, "Nameserver true TID: %d\n", tid);

	status = RegisterAs( "the_first" );
	bwprintf( COM2, "First registered with status: %d\n", status);

	tid = Create( 4, sub );
	bwprintf( COM2, "Sub() is created. TID: %d\n", tid );

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

//Testing first function
void test_task(){

	int sum;

	sum = TestCall(11,22,33,44,55,66);
	bwprintf( COM2, "The sum is:%d", sum);

	Exit();
}
