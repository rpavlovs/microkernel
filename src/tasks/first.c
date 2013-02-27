#include "userspace.h"

void idle() {
	debug( DBG_SYS, "IDLE: enters" );
	FOREVER {
		debug( DBG_SYS, "IDLE: idling" );
		Pass();
	}
}

#define CLOCK_TASK_PRIORITY		8


void first_task() {
	debug( DBG_SYS, "FIRST_TASK: start" );

	// ---------------------------------------------------------------------------------------------------------------------------------------
	// Servers Creation
	// ---------------------------------------------------------------------------------------------------------------------------------------
	debug( DBG_SYS, "FIRST_TASK: creating Nameserver" );
	int ns_tid = Create( NAMESERVER_TASK_PRIORITY, nameserver );
	assert( ns_tid == 1, "FIRST_TASK: Nameserver should have task id of 1" );

	debug( DBG_SYS, "FIRST_TASK: creating Timeserver" );
	int ts_tid = Create( TIMESERVER_TASK_PRIORITY, timeserver );

	debug( DBG_SYS, "FIRST_TASK: creating UART2 sender server" );
	int uart2_sender_tid = Create( UART2_SENDER_SERVER_PRIORITY, uart2_sender_server );

	debug( DBG_SYS, "FIRST_TASK: creating UART2 reciever server" );
	int uart2_receiver_tid = Create( UART2_RECEIVER_SERVER_PRIORITY, uart2_receiver_server );

	debug( DBG_SYS, "FIRST_TASK: creating Idle task" );
	int idle_tid = Create( 0, idle );

	debug( DBG_SYS, "FIRST_TASK: creating first user task" );
	int first_user_task_tid = Create( FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );
	
	// ---------------------------------------------------------------------------------------------------------------------------------------
	// Debug Statements
	// ---------------------------------------------------------------------------------------------------------------------------------------
	debug( DBG_SYS, "FIRST_TASK: setup is done." );
	debug( DBG_SYS, "FIRST_TASK: system debug level: %d ", DEBUG_LEVEL );

	debug( DBG_SYS, "FIRST_TASK: nameserver task id: %d, priority: %d, address: %d",
		ns_tid, NAMESERVER_TASK_PRIORITY, nameserver );

	debug( DBG_SYS, "FIRST_TASK: timeserver task id: %d, priority: %d, address: %d",
		ts_tid, TIMESERVER_TASK_PRIORITY, timeserver );

	debug( DBG_SYS, "FIRST_TASK: UART2 sender server task id: %d, priority: %d, address: %d",
		uart2_sender_tid, UART2_SENDER_SERVER_PRIORITY, uart2_sender_server );
	
	debug( DBG_SYS, "FIRST_TASK: UART2 reciever server task id: %d, priority: %d, address: %d",
		uart2_receiver_tid, UART2_RECEIVER_SERVER_PRIORITY, uart2_receiver_server );

	debug( DBG_SYS, "FIRST_TASK: sys idle task id  : %d, priority: %d, address: %d",
		idle_tid, 0, idle );
	
	debug( DBG_SYS, "FIRST_TASK: first user task id: %d, priority: %d, address: %d",
		first_user_task_tid, FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );

	debug( DBG_SYS, "FIRST_TASK: exit" );
	Exit();
	panic( "FIRST_TASK: Shoot the zombie!" );
}

