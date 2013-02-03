#include "commonspace.h"
#include "tasks/first.h"
#include "tasks/nameserver.h"
#include "tasks/RPS-game.h"

int
start_timer() {
    int *hi;
    hi = (int *)Timer4ValueHigh;
    *hi = (1 << 8);
    return 0;
}

long
get_time_ms() {
    int *timer_hi, *timer_lo;
    long cur_time;
    timer_hi = (int *)Timer4ValueHigh;
    timer_lo = (int *)Timer4ValueLow;
    cur_time = (*timer_lo + (*timer_hi << 8)) / 983;
    //cur_time = (*timer_lo + (*timer_hi << 8));
    return cur_time;
}

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

	start_timer();
	Create( 14, nameserver );

	int tid;
	// int sub_tid;
	//long start_time, end_time;
	//char buf64[64];
	//char buf4[4];
	//char reply_buf64[64];
	//char reply_buf4[4];

	//sub_tid = Create( 9, sub );

	//start_time = get_time_ms();
	//Send( sub_tid, buf4, 4, reply_buf4, 4 );
	//end_time = get_time_ms();

	//bwprintf( COM2, "It took %dms to send 4byte message\n", end_time - start_time );

	//start_time = get_time_ms();
	//Send( sub_tid, buf64, 64, reply_buf64, 64 );
	//end_time = get_time_ms();

	//bwprintf( COM2, "It took %dms to send 64byte message\n", end_time - start_time );

	tid = Create( 14, server_entry_point );

	tid = Create( 10, client_that_gets_bored );
	tid = Create( 10, client_always_play );

	debug( "First exits" );
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

void test_task(){

	int sum;

	sum = TestCall(11,22,33,44,55,66);
	bwprintf( COM2, "The sum is:%d", sum);

	Exit();
}
