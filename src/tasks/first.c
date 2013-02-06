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
	debug( DBG_SYS, "FIRST_TASK: start" );

	debug( DBG_SYS, "FIRST_TASK: creating Nameserver" );
	int ns_tid = Create( NAMESERVER_TASK_PRIORITY, nameserver );
	assert( ns_tid == 1, "FIRST_TASK: Nameserver should have task id of 1" );

	debug( DBG_SYS, "FIRST_TASK: creating Timeserver" );
	int ts_tid = Create( TIMESERVER_TASK_PRIORITY, timeserver );

	debug( DBG_SYS, "FIRST_TASK: creating first user task" );
	int first_user_task_tid = Create( FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );
	
	debug( DBG_SYS, "FIRST_TASK: setup is done." );
	debug( DBG_SYS, "FIRST_TASK: system debug level: %d ", DEBUG_LEVEL );
	debug( DBG_SYS, "FIRST_TASK: nameserver task id: %d, priority: %d, address: %d",
		ns_tid, NAMESERVER_TASK_PRIORITY, nameserver );
	debug( DBG_SYS, "FIRST_TASK: timeserver task id: %d, priority: %d, address: %d",
		ts_tid, TIMESERVER_TASK_PRIORITY, timeserver );
	debug( DBG_SYS, "FIRST_TASK: first user task id: %d, priority: %d, address: %d",
		first_user_task_tid, FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );

	debug( DBG_SYS, "FIRST_TASK: exit" );
	Exit();
	panic( "FIRST_TASK: Shoot the zombie!" );
}
