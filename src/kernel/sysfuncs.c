#include "kernelspace.h"

int
sys_create( int priority, void (*code) ( ), Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_CREATE: entered" );

	// ERROR: Scheduler was given a wrong task priority.
	if( priority < 0 || priority >= SCHED_NUM_PRIORITIES ) return -1;

	// Utility variables
	int new_tid;
	Task_descriptor *new_td;

	// Initialize utility variables
	new_tid = sched_get_free_tid( GLOBALS );
	new_td = &(GLOBALS->tasks[new_tid]);
	
	// Setup new task descriptor
	new_td->state = READY_TASK;
	new_td->priority = priority;
	new_td->lr = (int *)code;

	// Add new task descriptor to a proper scheduler queue
	sched_add_td( new_td, GLOBALS );

	// Rescheduling the task
	sys_reschedule( td, GLOBALS );

	return new_tid;
}

int
sys_mytid( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_MYTID: entered" );
	sys_reschedule( td, GLOBALS );
	return td->tid;
}

int
sys_myparenttid( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_MYPARENTTID: entered");
	sys_reschedule( td, GLOBALS );
	return td->parent_tid;
}

void
sys_pass(Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_PASS: entered" );
	sys_reschedule( td, GLOBALS );
}

void
sys_exit( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_EXIT: entered" );
	
	sched_remove_td( td, GLOBALS );

	// Updating the task's state
	td->state = ZOMBIE_TASK;
	GLOBALS->scheduler.tasks_exited++;
}

void
sys_reschedule( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, SCHEDULER_DEBUG_AREA, "SYS_RESCHEDULE: entered" );

	sched_remove_td( td, GLOBALS );
	sched_add_td( td, GLOBALS );

	// Updating the task's state
	td->state = READY_TASK;
}

int
sys_send( int receiver_tid, char *msg, int msglen, char *reply, int replylen,
		Task_descriptor *sender_td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_SEND: entered" );
	//todo_debug( 0x1, 4 );

	// Utility variables
	Task_descriptor *receiver_td;
	Message_queue *mailbox;

	// Utility variables initialization
	receiver_td = &(GLOBALS->tasks[receiver_tid]);
	mailbox = &(receiver_td->mailbox);

	//Send the message
	enqueue_msg_queue(sender_td->tid, msg, msglen, reply, replylen, mailbox);
	
	//Unblock the receiver
	sys_unblock_receive( receiver_td, GLOBALS );

	//Block the sender
	sender_td->state = SEND_TASK;
	sched_remove_td( sender_td, GLOBALS );

	//todo_debug( 0x5, 4 );

	return 0;
}

int
sys_receive( int *sender_tid, char *reciever_buf, const int reciever_buf_len,
			Task_descriptor *receiver_td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_RECEIVE: entered" );
	//todo_debug( 0x1, 5 );

	Message_queue *mailbox = &(receiver_td->mailbox);

	if( mailbox->size > 0 ) {
		bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_RECEIVE: Task not blocked. Mailbox not empty." );
		//todo_debug( 0x3, 5 );

		Send_info *send_info = &(mailbox->msg_infos[mailbox->oldest]);

		//Remove oldest message
		dequeue_msg_queue(mailbox);

		//Deliver the message to recievers buffer an provide the sender tid
		mem_cpy( send_info->msg, reciever_buf, send_info->msglen );
		*sender_tid = send_info->sender_tid;

		//Save information for a further reply
		Reply_info *reply_info = &(receiver_td->reply_infos[send_info->sender_tid]);
		reply_info->sender_tid = send_info->sender_tid;
		reply_info->reply = send_info->reply;
		reply_info->replylen = send_info->replylen;

		//Changing the state of the sender
		GLOBALS->tasks[send_info->sender_tid].state = REPLY_TASK;

		//Changing the state of the receiver
		sys_reschedule( receiver_td, GLOBALS );

		//todo_debug( 0x8, 5 );
	}
	else {
		bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_RECEIVE: Task blocked. Mailbox is empty" );
		//todo_debug( 0x9, 5 );

		//Save the current receive arguments
		Receive_info *receive_info = &(receiver_td->receive_info);
		receive_info->sender_tid = sender_tid;
		receive_info->msg = reciever_buf;
		receive_info->msglen = reciever_buf_len;

		//Remove the task from the READY queue
		receiver_td->state = RECEIVE_BLOCKED;
		sched_remove_td( receiver_td, GLOBALS );

		//todo_debug( 0x11, 5 );
	}
	return 0;
}

int
sys_reply( int sender_tid, char *reply, int replylen, Task_descriptor *receiver_td,
		Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_REPLY: entered [reply to %d from %d]",
			sender_tid, receiver_td->tid );
	//todo_debug( 0x1, 6 );

	Reply_info *reply_info = &(receiver_td->reply_infos[sender_tid]);

	mem_cpy( reply, reply_info->reply, replylen );

	Task_descriptor *sender_td = &(GLOBALS->tasks[sender_tid]);

	//Rescheduling the task
	sender_td->state = READY_TASK;
	sched_add_td( sender_td, GLOBALS );

	sys_reschedule( receiver_td, GLOBALS );

	return 0;
}

void
sys_unblock_receive( Task_descriptor *receiver_td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_UNBLOCK_RECEIVE: entered" );
	//todo_debug( 0x1, 7 );

	Message_queue *mailbox = &(receiver_td->mailbox);

	//todo_debug( 0x2, 7 );

	//The target task was waiting and there are SOME sends
	if( receiver_td->state != RECEIVE_BLOCKED || mailbox->size == 0 ) {
		//bwdebug( DBG_KERN, "SYS_UNBLOCK_RECEIVE: got called for a non-RECEIVE_BLOCKED "
		//	"or a task with no messages waiting to be received" );
		//todo_debug( 0x3, 7 );
		return;
	}

	//Modifying the queue
	Send_info *msg_info = &(mailbox->msg_infos[mailbox->oldest]);
	dequeue_msg_queue(mailbox);

	//Retrieve saved receive arguments
	Receive_info *receive_info = &(receiver_td->receive_info);
	mem_cpy( msg_info->msg, receive_info->msg, msg_info->msglen );
	*(receive_info->sender_tid) = msg_info->sender_tid;

	Reply_info *reply_info = &(receiver_td->reply_infos[msg_info->sender_tid]);
	reply_info->sender_tid = msg_info->sender_tid;
	reply_info->reply = msg_info->reply;
	reply_info->replylen = msg_info->replylen;

	//Changing the state of the sender
	GLOBALS->tasks[msg_info->sender_tid].state = REPLY_TASK;

	//RESCHEDULING///////////////////////////////////////
	//Unblocking the task
	receiver_td->state = READY_TASK;
	sched_add_td( receiver_td, GLOBALS );

	//todo_debug( 0x10, 7 );
}

int
sys_await_event( int eventid, int buffer_addr, Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_AWAIT_EVENT: entered. [event id: %d, task id: %d]",
		eventid, td->tid );
	bwassert( eventid < HWI_NUM_EVENTS, "SYS_AWAIT_EVENT: eventid is invalid" );
	
	//todo_debug( 0x1, 3 );
	GLOBALS->scheduler.hwi_watchers[eventid] = td;
	td->state = AWAIT_TASK;
	sched_remove_td( td, GLOBALS );
	
	// Save the event buffer information.
	td->event_char = buffer_addr; 
	//todo_debug( 0x2, 3 );
	
	// Reactivate interrupts
	// -> UART 1 -------------------------------------------------------------
	if ( eventid == UART1_INIT_SEND ) {
		//todo_debug( 0x4, 3 );
		////todo_debug( 51, 3 );
		// Reactivate both, transmit and modem status interrupts. 
		int *uart1_ctrl, temp; 
		uart1_ctrl = ( int * ) ( UART1_BASE + UART_CTLR_OFFSET ); 
		temp = *uart1_ctrl; 
		//*uart1_ctrl = temp | TIEN_MASK | MSIEN_MASK;
		*uart1_ctrl = temp | TIEN_MASK;
		//todo_debug( 0x5, 3 );
	}
	
	else if ( eventid == UART1_SEND_READY ) {
		//todo_debug( 0x6, 3 );
		////todo_debug( 52, 3 );
		// Reactivate the modem status interrupt. 
		int *uart1_ctrl, temp; 
		uart1_ctrl = ( int * ) ( UART1_BASE + UART_CTLR_OFFSET ); 
		temp = *uart1_ctrl; 
		//todo_debug( 0x7, 3 );
		//*uart1_ctrl = temp | MSIEN_MASK;
	}
	
	// -> UART 2 --------------------------------------------------------------
	else if ( eventid == UART2_SEND_READY ) {
		//todo_debug( 0x8, 3 );
		////todo_debug( 53, 3 );
		// Reactivate the transmit interrupt. 
		int *uart2_ctrl, temp; 
		uart2_ctrl = ( int * ) ( UART2_BASE + UART_CTLR_OFFSET ); 
		temp = *uart2_ctrl; 
		*uart2_ctrl = temp | TIEN_MASK; 
		//todo_debug( 0x9, 3 );
	}

	else {
		//Different interrupt!!!
		//todo_debug( 0x10, 3 );
	}
	
	return 0;
}

void
sys_get_system_data( System_data *data, Task_descriptor *td, Kern_Globals *GLOBALS ) {
	bwdebug( DBG_KERN, KERNEL_DEBUG_AREA, "SYS_GET_SYSTEM_DATA: entered. [caller task id: %d]", td->tid );
	data->active_tasks = GLOBALS->scheduler.tasks_alive;
	data->last_active_tid = GLOBALS->scheduler.last_active_tid;
	data->kernel_loop_cnt = GLOBALS->profdata.kernel_loop_cnt;
	data->cpu_utilization = GLOBALS->profdata.cpu_utilization;
	sys_reschedule( td, GLOBALS );
}

int
sys_testcall( int a, int b, int c, int d, int e, int f ) {
	return a + b + c + d + e + f;
}
