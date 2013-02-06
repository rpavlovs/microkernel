#include "userspace.h"

#define INT_CONTROL_BASE_1		0x800B0000		// VIC 1

// ----------------------------------------------------------------------------
// Debugging ( TODO: Delete this after making sure HWIs work )
// ----------------------------------------------------------------------------
void 
print_hwi_registers(){
	int *ptr = ( int * ) INT_CONTROL_BASE_1; 
	
	int i; 
	for ( i = 0; i < 9; i++ ){
		bwprintf( COM2, "PTR VALUE: BASE + %x REAL ADDRESS: %x VALUE: %x\n", i * 4, ptr, *ptr  ); 
		ptr += 1; 
	}
}

/**/
void first_task() {
	// Before:
	bwprintf( COM2, "Before:\n" ); 
	print_hwi_registers(); 
	start_timer(); 
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
 /**/ 
// ----------------------------------------------------------------------------
// Debugging END 
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// First Task
// ----------------------------------------------------------------------------
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
* */ 