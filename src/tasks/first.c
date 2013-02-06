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

#define INT_CONTROL_BASE_1		0x800B0000		// VIC 1

void 
print_hwi_registers2(){
	int *ptr = ( int * ) INT_CONTROL_BASE_1; 
	
	int i; 
	for ( i = 0; i < 9; i++ ){
		bwprintf( COM2, "PTR VALUE: BASE + %x REAL ADDRESS: %x VALUE: %x\n", i * 4, ptr, *ptr  ); 
		ptr += 1; 
	}
}

#define INITIAL_TIMER_LOAD 200
#define TIMER_ENABLE_FLAG 0x80 // 10000000
#define TIMER_MODE 0x40 // 1000000
#define MAX_SECONDS 59
#define MAX_MINUTES 59
#define MAX_DECI_SECONDS 9
#define TIMER_ROW_POS 2
#define TIMER_COL_POS 10

void start_timer2(){
	int timerControlValue; 
	int *timerLoad = ( int * ) TIMER1_BASE;
	//int *timerValue = ( int * ) ( TIMER1_BASE + VAL_OFFSET ); 
	int *timerControl = ( int * ) ( TIMER1_BASE + CRTL_OFFSET ); 

	// First the load is added. 
	*timerLoad = INITIAL_TIMER_LOAD;

	// The timer is enabled and configured.
	timerControlValue = *timerControl;
	timerControlValue = timerControlValue | TIMER_ENABLE_FLAG | TIMER_MODE;
	*timerControl = timerControlValue;
}

/**/
void first_task() {
	// Before:
	bwprintf( COM2, "Before:\n" ); 
	print_hwi_registers2(); 
	start_timer2(); 
	bwprintf( COM2, "After:\n" ); 
	//print_hwi_registers();
	bwprintf( COM2, "Bla1\n" ); 
	bwprintf( COM2, "Bla2\n" ); 
	bwprintf( COM2, "Bla3\n" ); 
	bwprintf( COM2, "Bla4\n" ); 
	bwprintf( COM2, "Bla5\n" ); 
	while( 1 ){
		bwprintf( COM2, "Bla\n" ); 	
	}
}

/**
void first_task() {
	debug( DBG_CURR_LVL, DBG_SYS, "FIRST_TASK: START %d\n", 1000 );
	
	int tid;
	tid = Create( 14, nameserver );
	assert( tid == 1, "Nameserver should have task id of 1" );

	debug( DBG_CURR_LVL, DBG_SYS, "FIRST_TASK: EXIT" );
	Exit();
}
*/

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













