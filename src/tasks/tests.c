#include <userspace.h>

//Testing SYSCALLS

void test_syscalls() {
	int sum;

	bwprintf( COM2, "Summing 11, 22,33,44,55,66...\n" );
	sum = TestCall( 11,22,33,44,55,66 );
	bwprintf( COM2, "The sum is: %d\n", sum );

	Exit();
}

//Testing NAMESERVER

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

void test_nameserver() {
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

//Testing TIMESERVER

void test_timeserver() {
	bwprintf( COM2, "test_timeserver entered\n" );
	int start_time, end_time;

	// Testing Time

	bwprintf( COM2, "It's %d ticks since timeserver was initialized.\n", Time() );

	bwprintf( COM2, "Passing to spend some time\n" );
	start_time = Time();
	Pass();
	end_time = Time();
	bwprintf( COM2, "Got back from passing, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );
	
	// Test Delay

	bwprintf( COM2, "Recording time and calling Delay( 10 )\n" );
	start_time = Time();
	Delay( 1 );
	end_time = Time();
	bwprintf( COM2, "Delay returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );
	
	bwprintf( COM2, "Recording time and calling Delay( 100 )\n" );
	start_time = Time();
	Delay( 10 );
	end_time = Time();
	bwprintf( COM2, "Delay returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );

	bwprintf( COM2, "Recording time and calling Delay( 1000 )\n" );
	start_time = Time();
	Delay( 100 );
	end_time = Time();
	bwprintf( COM2, "Delay returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );

	// Test DelayUntil

	bwprintf( COM2, "Recording time and calling DelayUntil( 1 )\n" );
	start_time = Time();
	DelayUntil( start_time + 1 );
	end_time = Time();
	bwprintf( COM2, "DelayUntill returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );

	bwprintf( COM2, "Recording time and calling DelayUntil( 10 )\n" );
	start_time = Time();
	DelayUntil( start_time + 10 );
	end_time = Time();
	bwprintf( COM2, "DelayUntill returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );

	bwprintf( COM2, "Recording time and calling DelayUntil( 100 )\n" );
	start_time = Time();
	DelayUntil( start_time + 100 );
	end_time = Time();
	bwprintf( COM2, "DelayUntill returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );
	
	bwprintf( COM2, "test_timeserver exits\n" );
	Exit();
}
