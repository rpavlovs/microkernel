#include "kernelspace.h"

int
sys_create( int priority, void (*code) ( ), Task_descriptor *td, Kern_Globals *GLOBALS ) {	
	debug( DBG_KERN, "SYS_CREATE: entered" );

	// ERROR: Scheduler  was given a wrong task priority.
	if( priority < 0 || priority >= SCHED_NUM_PRIORITIES ) return -1;
	
	// Getting the schedule
	Scheduler *sched = &(GLOBALS->scheduler);
	int new_tid;
	Task_descriptor *new_td;

	// Find a free task descriptor for a new task.
	new_tid = sched->last_issued_tid + 1;
	if( new_tid >= MAX_NUM_TASKS ) new_tid = 0;
	while( GLOBALS->tasks[new_tid].state != FREE_TASK ) {
		// ERROR: Scheduler  is out of task descriptors. 
		if( ++new_tid >= MAX_NUM_TASKS ) return -2;
	}

	//Updating the schedule
	sched->last_issued_tid = new_tid;
	sched->tasks_alive++;
	
	// Setup new task descriptor
	new_td = &(GLOBALS->tasks[new_tid]);
	new_td->state = READY_TASK;
	new_td->priority = priority;
	new_td->lr = (int *)code;

	// Add new task descriptor to a proper scheduler queue
	Task_queue *queue = &(sched->queues[priority]);

	assert( queue->size < SCHED_QUEUE_LENGTH, "SYS_CREATE: Scheduler  queue must not be full" );

	// If the queue is empty or the newest pointer is at the end of the td_ptrs buffer
	// put the next td_ptr at the beginning on the buffer  
	if (queue->size == 0 || ++(queue->newest) >= SCHED_QUEUE_LENGTH) queue->newest = 0;
	
	// If the queue was empty then newest and oldest elements are the same 
	// and are at the beginning of the buffer
	if (queue->size == 0) queue->oldest = 0;

	// Updating the queue
	queue->size++;
	queue->td_ptrs[queue->newest] = new_td;

	// Rescheduling the task
	sys_reschedule( td, GLOBALS );

	return new_tid;
}

int
sys_mytid( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_MYTID: entered" );
	sys_reschedule( td, GLOBALS );
	return td->tid;
}

int
sys_myparenttid( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_MYPARENTTID: entered");
	sys_reschedule( td, GLOBALS );
	return td->parent_tid;
}

void
sys_pass(Task_descriptor *td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_PASS: entered" );
	sys_reschedule( td, GLOBALS );
}

void
sys_exit( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_EXIT: entered" );
	
	// Removing the first task from the corresponding queue
	Task_queue *queue = &(GLOBALS->scheduler.queues[td->priority]);
	if( ++(queue->oldest) >= SCHED_QUEUE_LENGTH )
		queue->oldest = 0;
	queue->size--;

	// Updating the task's state
	td->state = ZOMBIE_TASK;

	// Updating the schedule
	GLOBALS->scheduler.tasks_alive--;
	GLOBALS->scheduler.tasks_exited++;
}

void
sys_reschedule( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_RESCHEDULE: entered" );

	// Getting the priority queue
	Task_queue *queue = &(GLOBALS->scheduler.queues[td->priority]);
	assert( queue->td_ptrs[queue->oldest] == td, "can only reschelude most recent task" );
	if( queue->size > 1 ) {
		// Removing the first task from the queue
		if( ++(queue->oldest) >= SCHED_QUEUE_LENGTH ) queue->oldest = 0;

		// Adding the task to the end of the queue
		if( ++(queue->newest) >= SCHED_QUEUE_LENGTH ) queue->newest = 0;
		queue->td_ptrs[queue->newest] = td;
	}

	// Updating the task's state
	td->state = READY_TASK;
}

int
sys_send( int receiver_tid, char *msg, int msglen, char *reply, int replylen,
		Task_descriptor *sender_td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_SEND: entered" );

	//Getting TD of the target task
	Task_descriptor *receiver_td = &(GLOBALS->tasks[receiver_tid]);

	//Getting the receive queue of the target task
	Message_queue *mailbox = &(receiver_td->mailbox);

	assert( mailbox->size < MAX_NUM_TASKS - 1,
		"Waiting to be recieved queue should not overflow :(" );

	//Modifying the queue
	mailbox->size++;
	if( ++(mailbox->newest) == MAX_NUM_TASKS ) {
		mailbox->newest = 0;
	}

	Send_info *msg_info = &(mailbox->msg_infos[mailbox->newest]);
	msg_info->sender_tid = sender_td->tid;
	msg_info->msg = msg;
	msg_info->msglen = msglen;
	msg_info->reply = reply;
	msg_info->replylen = replylen;
	

	//Unblocking the target task
	sys_unblock_receive( receiver_td, GLOBALS );

	//BLOCKING///////////////////////////////////////////////////
	//Change the state of the calling task to SEND_BLOCKED
	sender_td->state = SEND_TASK;
	GLOBALS->scheduler.tasks_alive--;
	
	//Remove the task from the READY queue
	Task_queue *pqueue = &(GLOBALS->scheduler.queues[sender_td->priority]);
	dequeue_tqueue(pqueue);

	return 0;
}

int
sys_receive( int *sender_tid, char *reciever_buf, const int reciever_buf_len,
	Task_descriptor *receiver_td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_RECEIVE: entered" );

	Message_queue *mailbox = &(receiver_td->mailbox);

	if( mailbox->size > 0 ) {
		debug( DBG_KERN, "SYS_RECEIVE: Task not blocked. Mailbox not empty." );

		Send_info *send_info = &(mailbox->msg_infos[mailbox->oldest]);

		//Remove oldest message
		if( ++(mailbox->oldest) == MAX_NUM_TASKS ) mailbox->oldest = 0;
		mailbox->size--;

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
	} else {
		debug( DBG_KERN, "SYS_RECEIVE: Task blocked. Mailbox is empty" );

		//Save the current receive arguments
		Receive_info *receive_info = &(receiver_td->receive_info);
		receive_info->sender_tid = sender_tid;
		receive_info->msg = reciever_buf;
		receive_info->msglen = reciever_buf_len;

		//Remove the task from the READY queue
		dequeue_tqueue( &(GLOBALS->scheduler.queues[receiver_td->priority]) );

		receiver_td->state = RECEIVE_BLOCKED;

		GLOBALS->scheduler.tasks_alive--;
	}
	return 0;
}

int
sys_reply( int sender_tid, char *reply, int replylen, Task_descriptor *receiver_td,
		Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_REPLY: entered [reply to %d from %d]",
		sender_tid, receiver_td->tid );

	Reply_info *reply_info = &(receiver_td->reply_infos[sender_tid]);

	mem_cpy( reply, reply_info->reply, replylen );

	Task_descriptor *sender_td = &(GLOBALS->tasks[sender_tid]);

	//Rescheduling the task
	sender_td->state = READY_TASK;
	GLOBALS->scheduler.tasks_alive++;

	Task_queue *pqueue = &(GLOBALS->scheduler.queues[sender_td->priority]);
	enqueue_tqueue( sender_td, pqueue );

	sys_reschedule( receiver_td, GLOBALS );
	return 0;
}

void
sys_unblock_receive( Task_descriptor *receiver_td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_UNBLOCK_RECEIVE: entered" );

	Message_queue *mailbox = &(receiver_td->mailbox);

	//The target task was waiting and there are SOME sends
	if( receiver_td->state != RECEIVE_BLOCKED || mailbox->size == 0 ) {
		debug( DBG_KERN, "SYS_UNBLOCK_RECEIVE: got called for a non-RECEIVE_BLOCKED "
			"or a task with no messages waiting to be received" );
		return;
	}

	//Modifying the queue
	mailbox->size--;
	Send_info *msg_info = &(mailbox->msg_infos[mailbox->oldest]);
	if( ++(mailbox->oldest) == MAX_NUM_TASKS ) {
		mailbox->oldest = 0;
	}

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
	GLOBALS->scheduler.tasks_alive++;

	//Rescheduling the task
	Task_queue *pqueue = &(GLOBALS->scheduler.queues[receiver_td->priority]);
	enqueue_tqueue(receiver_td, pqueue);
}

int
sys_await_event( int eventid, int buffer_addr, Task_descriptor *td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_AWAIT_EVENT: entered. [event id: %d, task id: %d]",
		eventid, td->tid );
	assert( eventid < HWI_NUM_EVENTS, "SYS_AWAIT_EVENT: eventid is invalid" );
	
	GLOBALS->scheduler.hwi_watchers[eventid] = td;
	
	// Remove the task from the READY queue
	td->state = AWAIT_TASK;
	Task_queue *pqueue = &(GLOBALS->scheduler.queues[td->priority]);
	dequeue_tqueue(pqueue);
	GLOBALS->scheduler.tasks_alive--;
	
	// Save the event buffer information.
	todo_debug( buffer_addr, 1 );
	td->event_char = buffer_addr; 
	
	// Reactivate interrupts
	// -> UART 1 -------------------------------------------------------------
	//todo_debug( 50, 2 );
	//todo_debug( eventid, 2 );
	if ( eventid == UART1_INIT_SEND ) {
		//todo_debug( 51, 2 );
		// Reactivate both, transmit and modem status interrupts. 
		int *uart1_ctrl, temp; 
		uart1_ctrl = ( int * ) ( UART1_BASE + UART_CTLR_OFFSET ); 
		temp = *uart1_ctrl; 
		*uart1_ctrl = temp | TIEN_MASK | MSIEN_MASK;
	}
	
	if ( eventid == UART1_SEND_READY ) {
		//todo_debug( 52, 2 );
		// Reactivate the modem status interrupt. 
		int *uart1_ctrl, temp; 
		uart1_ctrl = ( int * ) ( UART1_BASE + UART_CTLR_OFFSET ); 
		temp = *uart1_ctrl; 
		*uart1_ctrl = temp | MSIEN_MASK;
	}
	
	// -> UART 2 --------------------------------------------------------------
	if ( eventid == UART2_SEND_READY ) {
		//todo_debug( 53, 2 );
		// Reactivate the transmit interrupt. 
		int *uart2_ctrl, temp; 
		uart2_ctrl = ( int * ) ( UART2_BASE + UART_CTLR_OFFSET ); 
		temp = *uart2_ctrl; 
		*uart2_ctrl = temp | TIEN_MASK; 
	}
	
	return 0;
}

int
sys_testcall( int a, int b, int c, int d, int e, int f ) {
//int sys_testcall(int a, int b, int c, int d, int e){ //, int f){
//int sys_testcall(int a, int b, int c, int d){

	return a + b + c + d + e + f;
}
