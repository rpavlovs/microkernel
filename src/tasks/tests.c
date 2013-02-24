#include <userspace.h>

//Testing SYSCALLS

#define INT_CONTROL_BASE_1		0x800B0000		// VIC 1

void task_test_uart1(){
	int uart1_receiver_tid = Create( 14, uart1_receiver_server );
	
	
}

void task_test_uart2() {	
	// CREATE THE UART2 RECEIVER SERVER. 
	debug( DBG_SYS, "FIRST_TASK: creating UART2 Receiver Server." );
	todo_debug( 0, 0 );
	todo_debug( 0, 1 );
	todo_debug( 0, 2 );
	
	int uart2_sender_tid = Create( 14, uart2_sender_server );
	int uart2_receiver_tid = Create( 14, uart2_receiver_server );
	
	char c = Getc( COM2 );
	char d = Getc( COM2 );
	
	Putc( COM2, c );
	Putc( COM2, d );
	Putc( COM2, 'z' );
	
	//bwprintf( COM2, "\nEVERYTHING THAT HAS A BEGINNING HAS AN END, NEO!!!");
	
	Exit(); 
}


void 
print_hwi_registers(){
	int *ptr = ( int * ) INT_CONTROL_BASE_1; 
	
	int i; 
	for ( i = 0; i < 9; i++ ){
		bwprintf( COM2, "PTR VALUE: BASE + %x REAL ADDRESS: %x VALUE: %x\n", i * 4, ptr, *ptr  ); 
		ptr += 1; 
	}
}

void test_syscalls() {
	int sum;

	bwprintf( COM2, "Summing 11, 22,33,44,55,66...\n" );
	sum = TestCall( 11,22,33,44,55,66 );
	bwprintf( COM2, "The sum is: %d\n", sum );

	Exit();
}

// void test_awaitEvent() {
	
// 	// Before:
// 	bwprintf( COM2, "Before:\n" ); 
// 	print_hwi_registers(); 
// 	start_timer(); 
// 	bwprintf( COM2, "After:\n" ); 
// 	//print_hwi_registers();
// 	bwprintf( COM2, "Bla1\n" ); 
// 	bwprintf( COM2, "Bla2\n" ); 
// 	bwprintf( COM2, "Bla3\n" ); 
// 	bwprintf( COM2, "Bla4\n" ); 
// 	bwprintf( COM2, "Bla5\n" ); 
// 	while( 1 ){
// 		bwprintf( COM2, "Bla\n" ); 	
// 	}
// }

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

	bwprintf( COM2, "Recording time and calling Delay( 1 )\n" );
	start_time = Time();
	Delay( 1 );
	end_time = Time();
	bwprintf( COM2, "Delay returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );
	
	bwprintf( COM2, "Recording time and calling Delay( 10 )\n" );
	start_time = Time();
	Delay( 10 );
	end_time = Time();
	bwprintf( COM2, "Delay returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );

	bwprintf( COM2, "Recording time and calling Delay( 100 )\n" );
	start_time = Time();
	Delay( 100 );
	end_time = Time();
	bwprintf( COM2, "Delay returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );

	bwprintf( COM2, "Recording time and calling Delay( 1000 )\n" );
	start_time = Time();
	Delay( 1000 );
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

	bwprintf( COM2, "Recording time and calling DelayUntil( 1000 )\n" );
	start_time = Time();
	DelayUntil( start_time + 1000 );
	end_time = Time();
	bwprintf( COM2, "DelayUntill returned, current time is %d. Process was delayed for %d\n",
		end_time, end_time - start_time );
	
	bwprintf( COM2, "test_timeserver exits\n" );
	// while(1);
	Exit();
}


