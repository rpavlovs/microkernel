#include "userspace.h"

void idle() {
	bwdebug( DBG_SYS, "IDLE: enters" );
	FOREVER {
		bwdebug( DBG_SYS, "IDLE: idling..." );
		Pass();
	}
}

#define CLOCK_TASK_PRIORITY		8


void first_task() {
	bwdebug( DBG_SYS, "FIRST_TASK: start" );

	// ---------------------------------------------------------------------------------------
	// Servers Creation
	// ---------------------------------------------------------------------------------------
	bwdebug( DBG_SYS, "FIRST_TASK: creating Nameserver" );
	int ns_tid = Create( NAMESERVER_TASK_PRIORITY, nameserver );
	bwassert( ns_tid == 1, "FIRST_TASK: Nameserver should have task id of 1" );


	bwdebug( DBG_SYS, "FIRST_TASK: creating Timeserver" );
	int ts_tid = Create( TIMESERVER_TASK_PRIORITY, timeserver );

	bwdebug( DBG_SYS, "FIRST_TASK: creating UART1 sender server" );
	int uart1_sender_tid = Create( UART1_SENDER_SERVER_PRIORITY, uart1_sender_server );
	
	bwdebug( DBG_SYS, "FIRST_TASK: creating UART1 reciever server" );
	int uart1_receiver_tid = Create( UART1_RECEIVER_SERVER_PRIORITY, uart1_receiver_server );

	bwdebug( DBG_SYS, "FIRST_TASK: creating UART2 sender server" );
	int uart2_sender_tid = Create( UART2_SENDER_SERVER_PRIORITY, uart2_sender_server );

	bwdebug( DBG_SYS, "FIRST_TASK: creating UART2 reciever server" );
	int uart2_receiver_tid = Create( UART2_RECEIVER_SERVER_PRIORITY, uart2_receiver_server );

	bwdebug( DBG_SYS, "FIRST_TASK: creating command server" );
	int cmd_server_tid = Create( COMMAND_SERVER_PRIORITY, commandserver );  
	
//	bwdebug( DBG_SYS, "FIRST_TASK: creating sensor server" );
//	int sensor_server_tid = Create( SENSOR_SERVER_PRIORITY, sensors_server );
//	
//	bwdebug( DBG_SYS, "FIRST_TASK: creating switches server" );
//	int switches_server_tid = Create( SWITCHES_SERVER_PRIORITY, switchserver );
	
	bwdebug( DBG_SYS, "FIRST_TASK: creating Idle task" );
	int idle_tid = Create( 0, idle );

	bwdebug( DBG_SYS, "FIRST_TASK: creating first user task" );
	int first_user_task_tid = Create( FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );
	
	// ---------------------------------------------------------------------------------------
	// Debug Statements
	// ---------------------------------------------------------------------------------------
	bwdebug( DBG_SYS, "FIRST_TASK: setup is done." );
	bwdebug( DBG_SYS, "FIRST_TASK: system bwdebug level: %d ", DEBUG_LEVEL );

	bwdebug( DBG_SYS, "FIRST_TASK: nameserver task id: %d, priority: %d, address: %d",
		ns_tid, NAMESERVER_TASK_PRIORITY, nameserver );

	bwdebug( DBG_SYS, "FIRST_TASK: timeserver task id: %d, priority: %d, address: %d",
		ts_tid, TIMESERVER_TASK_PRIORITY, timeserver );

	bwdebug( DBG_SYS, "FIRST_TASK: UART1 sender server task id: %d, priority: %d, address: %d",
		uart1_sender_tid, UART1_SENDER_SERVER_PRIORITY, uart1_sender_server );
	
	bwdebug( DBG_SYS, "FIRST_TASK: UART1 reciever server task id: %d, priority: %d, address: %d",
		uart1_receiver_tid, UART1_RECEIVER_SERVER_PRIORITY, uart1_receiver_server );

	bwdebug( DBG_SYS, "FIRST_TASK: UART2 sender server task id: %d, priority: %d, address: %d",
		uart2_sender_tid, UART2_SENDER_SERVER_PRIORITY, uart2_sender_server );
	
	bwdebug( DBG_SYS, "FIRST_TASK: UART2 reciever server task id: %d, priority: %d, address: %d",
		uart2_receiver_tid, UART2_RECEIVER_SERVER_PRIORITY, uart2_receiver_server );
	
	bwdebug( DBG_SYS, "FIRST_TASK: UART2 sender server task id: %d, priority: %d, address: %d",
		uart1_sender_tid, UART1_SENDER_SERVER_PRIORITY, uart1_sender_server );
	
	bwdebug( DBG_SYS, "FIRST_TASK: UART2 reciever server task id: %d, priority: %d, address: %d",
		uart1_receiver_tid, UART1_RECEIVER_SERVER_PRIORITY, uart1_receiver_server );

	bwdebug( DBG_SYS, "FIRST_TASK: command server task id  : %d, priority: %d, address: %d",
		cmd_server_tid, COMMAND_SERVER_PRIORITY, commandserver );

//	bwdebug( DBG_SYS, "FIRST_TASK: sensor server task id  : %d, priority: %d, address: %d",
//		sensor_server_tid, SENSOR_SERVER_PRIORITY, sensors_server );
//		
//	bwdebug( DBG_SYS, "FIRST_TASK: switches server task id  : %d, priority: %d, address: %d",
//		switches_server_tid, SWITCHES_SERVER_PRIORITY, switchserver );
	
	bwdebug( DBG_SYS, "FIRST_TASK: sys idle task id  : %d, priority: %d, address: %d",
		idle_tid, 0, idle );
	
	bwdebug( DBG_SYS, "FIRST_TASK: first user task id: %d, priority: %d, address: %d",
		first_user_task_tid, FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );

	bwdebug( DBG_SYS, "FIRST_TASK: exit" );
	Exit();
	bwpanic( "FIRST_TASK: Shoot the zombie!" );
}

