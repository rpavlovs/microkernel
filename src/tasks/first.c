#include "userspace.h"

void idle() {
	debug( DBG_SYS, "IDLE: enters" );
	FOREVER {
		// debug( DBG_SYS, "IDLE: idling..." );
		Pass();
	}
}

#define CLOCK_TASK_PRIORITY		8
void first_task(){
	// TODO: Remove this task.
	
	/*
	 // TEST TO GET THE ADDRESS TO USE FOR dump
	int *ptr = ( int * ) ( 0x01dffe50 );
	int *ptr2 = ( int * ) ( 0x01dffe54 );
	*ptr = 50;
	*ptr2 = 100; 
	bwprintf( COM2, " Memory address: %x Value: %d Hex Value: %x \n", ptr, *ptr, *ptr );
	bwprintf( COM2, " Memory address: %x Value: %d Hex Value: %x \n", ptr2, *ptr2, *ptr2 );
	*/
	
	/*
	// TEST TO MAKE SURE BASIC UART INTERRUPTS ARE RUNNING. 
	todo_debug( 1, 0 );
	AwaitEvent( UART2_SEND_READY, 0, 0 );
	todo_debug( 100, 0 );
	 */
	
	/*
	// TEST TO MAKE SURE THAT THE CLOCK IS WORKING. 
	debug( DBG_SYS, "FIRST_TASK: creating Nameserver" );
	int ns_tid = Create( NAMESERVER_TASK_PRIORITY, nameserver );
	assert( ns_tid == 1, "FIRST_TASK: Nameserver should have task id of 1" );
	
	debug( DBG_SYS, "FIRST_TASK: creating Timeserver" );
	int ts_tid = Create( TIMESERVER_TASK_PRIORITY, timeserver );
	
	// Create the idle task. (Will execute when no other task has anything to do). 
	// TODO: Make the idle task gather statistics. 
	int idle_tid = Create( 0, idle );
	
	// Create the task that will deal with the clock. 
	debug( DBG_SYS, "FIRST_TASK: Creating clock-task" );
	int clk_tid = Create( CLOCK_TASK_PRIORITY, draw_clock ); 
	
	Exit();
	panic( "FIRST_TASK: Shoot the zombie!" );
	*/
	
	// TEST TO MAKE SURE THAT UARTS ARE WORKING PROPERLY
	debug( DBG_SYS, "FIRST_TASK: creating Nameserver" );
	int ns_tid = Create( NAMESERVER_TASK_PRIORITY, nameserver );
	assert( ns_tid == 1, "FIRST_TASK: Nameserver should have task id of 1" );
	
	// CREATE THE UART2 RECEIVER SERVER. 
	debug( DBG_SYS, "FIRST_TASK: creating UART2 Receiver Server." );
	todo_debug( 0, 1 );
	todo_debug( 18, 0 );
	int uart2_receiver_tid = Create( 14, uart2_receiver_server );
	//todo_debug( uart2_receiver_tid, 1 );
	todo_debug( 19, 0 );
	
	// Checking if we can indeed send characters. 
	todo_debug( 20, 0 );
	char c = Getc( COM2 );
	//todo_debug( 21, 0 );
	
	//todo_debug( c, 1 );
}

/*
void first_task() {
	debug( DBG_SYS, "FIRST_TASK: start" );

	debug( DBG_SYS, "FIRST_TASK: creating Nameserver" );
	int ns_tid = Create( NAMESERVER_TASK_PRIORITY, nameserver );
	assert( ns_tid == 1, "FIRST_TASK: Nameserver should have task id of 1" );

	debug( DBG_SYS, "FIRST_TASK: creating Timeserver" );
	int ts_tid = Create( TIMESERVER_TASK_PRIORITY, timeserver );

	debug( DBG_SYS, "FIRST_TASK: creating Idle task" );
	int idle_tid = Create( 0, idle );

	debug( DBG_SYS, "FIRST_TASK: creating first user task" );
	int first_user_task_tid = Create( FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );
	
	debug( DBG_SYS, "FIRST_TASK: setup is done." );
	debug( DBG_SYS, "FIRST_TASK: system debug level: %d ", DEBUG_LEVEL );
	debug( DBG_SYS, "FIRST_TASK: nameserver task id: %d, priority: %d, address: %d",
		ns_tid, NAMESERVER_TASK_PRIORITY, nameserver );
	debug( DBG_SYS, "FIRST_TASK: timeserver task id: %d, priority: %d, address: %d",
		ts_tid, TIMESERVER_TASK_PRIORITY, timeserver );
	debug( DBG_SYS, "FIRST_TASK: sys idle task id  : %d, priority: %d, address: %d",
		idle_tid, 1, idle );
	debug( DBG_SYS, "FIRST_TASK: first user task id: %d, priority: %d, address: %d",
		first_user_task_tid, FIRST_USER_TASK_PRIORITY, FIRST_USER_TASK_NAME );

	debug( DBG_SYS, "FIRST_TASK: exit" );
	Exit();
	panic( "FIRST_TASK: Shoot the zombie!" );
}
*/
