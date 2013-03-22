#include <userspace.h>

//Testing SYSCALLS

#define INT_CONTROL_BASE_1		0x800B0000		// VIC 1
#define INT_CONTROL_BASE_2		0x800C0000		// VIC 2
#define INT_ENABLE_OFFSET		0x10
#define INT_RESET_VALUE			0x0

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Stress Tests
// -- These tests are longer, and more time consuming.
// -----------------------------------------------------------------------------------------------------------------------------------------------
void stress_test_uart1_getc(){
	FOREVER {
		char c = Getc( COM1 );
		Putc( COM2, c );
	}
	
	Exit();
}

void stress_test_uart2_getc(){
	while(1){
		char c = Getc( COM2 ); 
		bwprintf( COM2, "%c", c ); 
	}
	
	Exit();
}

void stress_test_uart2_putc(){
	while(1){
		Putc( COM2, 'y' ); 
	}
	
	Exit();	
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Simple Tests
// -- These tests are similar to unit tests, since they are very small and run fast. 
// -----------------------------------------------------------------------------------------------------------------------------------------------

void test_debug(){
	// TURN INTERRUPTS OFF
	int *vic1EnablePointer = ( int * )( INT_CONTROL_BASE_1 + INT_ENABLE_OFFSET );
	int *vic2EnablePointer = ( int * )( INT_CONTROL_BASE_2 + INT_ENABLE_OFFSET );
	
	int initialInterruptsVIC1 = INT_RESET_VALUE;
	int initialInterruptsVIC2 = INT_RESET_VALUE;

	*vic1EnablePointer = initialInterruptsVIC1; 
	*vic2EnablePointer = initialInterruptsVIC2; 

	// Show the flag values
	bwprintf( COM2, "FLAG: KERNEL_DEBUG_AREA Value: %d\n", KERNEL_DEBUG_AREA );				// Should be 1
	bwprintf( COM2, "FLAG: SCHEDULER_DEBUG_AREA Value: %d\n", SCHEDULER_DEBUG_AREA );		// Should be 2
	bwprintf( COM2, "FLAG: HWI_DEBUG_AREA Value: %d\n", HWI_DEBUG_AREA );					// Should be 4
	bwprintf( COM2, "FLAG: RPS_GAME_DEBUG_AREA Value: %d\n", RPS_GAME_DEBUG_AREA );			// Should be ?

	// Print a value that should be executed
	bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "TEST1: This should show.\n" );
	bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "TEST2: This should show.\n" );

	// Print a value that shouldn't be executed. 
	bwdebug( DBG_SYS, TIMESERVER_DEBUG_AREA, "TEST: This should NOT show.\n" );
	bwdebug( DBG_SYS, TIMESERVER_DEBUG_AREA, "TEST: This should NOT show.\n" );

	bwprintf( COM2, "VALUE TIMESERVER: %d \n", ( DEBUG_AREAS ) & ( TIMESERVER_DEBUG_AREA ) ); 
	bwprintf( COM2, "VALUE: %d \n", ( DEBUG_AREAS ) & ( UART1_SENDER_DEBUG_AREA ) ); 
}

void test_sensors_server(){

	bwprintf( COM2, "Sunshine Before! :)");
	Create( 14, sensors_server ); 
	bwprintf( COM2, "Sunshine After! :)");
	
	Exit();
}

void test_user_dashboard(){
	int dashboard_tid = Create( 10, user_dashboard ); 
	bwprintf( COM2, "Dashboard task created: %d", dashboard_tid );
	
	Exit(); 
}

void test_uart1_send(){
	int train_number = 35;
	
	//Putc( COM2, 'A' );
	Putc( COM1, 14 );
	Delay( 1 );
	Putc( COM1, train_number );
	
	Putc( COM2, 'B' );
	Delay( 1000 ); 
	
	Putc( COM2, 'C' );
	Putc( COM1, 0 );
	Delay( 1 );
	Putc( COM1, train_number );
	Putc( COM2, 'D' );
	
	Exit();
}

void test_uart1_receive(){
	while(1){
		char c = Getc( COM1 );
		bwprintf( COM2, "%c", c ); 
	}
	
	Exit();
}

void task_test_uart2() {	
	// CREATE THE UART2 RECEIVER SERVER. 
	bwdebug( DBG_SYS, TESTS_DEBUG_AREA, "FIRST_TASK: creating UART2 Receiver Server." );
	////todo_debug( 0, 0 );
	////todo_debug( 0, 1 );
	////todo_debug( 0, 2 );
	
	//int uart2_sender_tid = Create( 14, uart2_sender_server );
	//int uart2_receiver_tid = Create( 14, uart2_receiver_server );
	
	char c = Getc( COM2 );
	char d = Getc( COM2 );
	
	Putc( COM2, c );
	Putc( COM2, d );
	Putc( COM2, 'z' );
		
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

void sub_delay_1() {
	bwprintf( COM2, "sub_delay_1: enter. [time: %d]\n", Time() );
	int start_time, end_time;
	
	start_time = Time();
	Delay( 1 );
	end_time = Time();

	bwprintf( COM2, "sub_delay_1: delay took %d; exiting. [time: %d]\n",
		 end_time - start_time, end_time );
	Exit();
}

void sub_delay_10() {
	bwprintf( COM2, "sub_delay_10: enter. [time: %d]\n", Time() );
	int start_time, end_time;
	
	start_time = Time();
	Delay( 10 );
	end_time = Time();

	bwprintf( COM2, "sub_delay_10: delay took %d; exiting. [time: %d]\n",
		 end_time - start_time, end_time );
	Exit();
}

void sub_delay_100() {
	bwprintf( COM2, "sub_delay_100: enter. [time: %d]\n", Time() );
	int start_time, end_time;
	
	start_time = Time();
	Delay( 100 );
	end_time = Time();

	bwprintf( COM2, "sub_delay_100: delay took %d; exiting. [time: %d]\n",
		 end_time - start_time, end_time );
	Exit();
}

void sub_delay_1000() {
	bwprintf( COM2, "sub_delay_1000: enter. [time: %d]\n", Time() );
	int start_time, end_time;
	
	start_time = Time();
	Delay( 1000 );
	end_time = Time();

	bwprintf( COM2, "sub_delay_1000: delay took %d; exiting. [time: %d]\n",
		 end_time - start_time, end_time );
	Exit();
}

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
	
	bwprintf( COM2, "Starting background delay routines\n" ); 
	Create( 10, sub_delay_1 );
	Create( 10, sub_delay_10 );
	Create( 10, sub_delay_100 );
	Create( 10, sub_delay_1000 );
	bwprintf( COM2, "Done creating background delay routines\n" );

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


