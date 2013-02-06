#include "kernelspace.h"

int
sys_create( int priority, void (*code) ( ), Task_descriptor *td, Kern_Globals *GLOBALS ) {	
	debug( DBG_KERN, "SYS_CREATE: entered" );

	// ERROR: Scheduler was given a wrong task priority.
	if( priority < 0 || priority >= SCHED_NUM_PRIORITIES ) return -1;
	
	// Getting the schedule
	Schedule *sched = &(GLOBALS->schedule);
	int new_tid;
	Task_descriptor *new_td;

	// Find a free task descriptor for a new task.
	new_tid = sched->last_issued_tid + 1;
	if( new_tid >= MAX_NUM_TASKS ) new_tid = 0;
	while( GLOBALS->tasks[new_tid].state != FREE_TASK ) {
		// ERROR: Scheduler is out of task descriptors. 
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
	Task_queue *queue = &(sched->priority[priority]);

	assert( queue->size < SCHED_QUEUE_LENGTH, "SYS_CREATE: Scheduler queue must not be full" );

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

	// Getting task properties
	int priority = td->priority;
	
	// Getting the schedule
	Schedule *sched = &(GLOBALS->schedule);
	Task_queue *pqueue = &(sched->priority[priority]);

	// Removing the first task from the queue
	if (++(pqueue->oldest) >= SCHED_QUEUE_LENGTH) pqueue->oldest = 0;

	// Updating the task's state
	td->state = ZOMBIE_TASK;

	// Updating the queue
	pqueue->size--;

	// Updating the schedule
	sched->tasks_alive--;
}

void
sys_reschedule( Task_descriptor *td, Kern_Globals *GLOBALS ) {
	debug( DBG_KERN, "SYS_RESCHEDULE: entered" );

	// Getting task properties
	int priority = td->priority;
	
	// Getting the schedule
	Schedule *sched = &(GLOBALS->schedule);
	// Getting the priority queue
	Task_queue *pqueue = &(sched->priority[priority]);

	//If there are more than one task in the queue
	if(pqueue->size > 1)
	{
		// Removing the first task from the queue
		if (++(pqueue->oldest) >= SCHED_QUEUE_LENGTH) pqueue->oldest = 0;

		// Adding the task to the end of the queue
		if (++(pqueue->newest) >= SCHED_QUEUE_LENGTH) pqueue->newest = 0;
		pqueue->td_ptrs[pqueue->newest] = td;
	}

	// Updating the task's state
	td->state = READY_TASK;
}

int
sys_send( int receiver_tid, char *msg, int msglen, char *reply, int replylen,
		Task_descriptor *sender_td, Kern_Globals *GLOBALS ) {

	//Getting TD of the target task
	Task_descriptor *receiver_td = &(GLOBALS->tasks[receiver_tid]);

	//Getting the receive queue of the target task
	Message_queue *receive_queue = &(receiver_td->receive_queue);

	assert( receive_queue->size < MAX_NUM_TASKS - 1,
		"Waiting to be recieved queue should not overflow :(" );

	//Modifying the queue
	receive_queue->size++;
	if( ++(receive_queue->newest) == MAX_NUM_TASKS ) {
		receive_queue->newest = 0;
	}

	Message_info *msg_info = &(receive_queue->msg_infos[receive_queue->newest]);
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
	GLOBALS->schedule.tasks_alive--;
	
	//Remove the task from the READY queue
	Task_queue *pqueue = &(GLOBALS->schedule.priority[sender_td->priority]);
	dequeue_tqueue(pqueue);

	return 0;
}

int
sys_receive( int *sender_tid, char *msg, int msglen, Task_descriptor *receiver_td,
		Kern_Globals *GLOBALS ) {

	Message_queue *receive_queue = &(receiver_td->receive_queue);
	//If there are SOME sends from other tasks to the current task
	if( receive_queue->size > 0 ) {
		debug( DBG_KERN, "SYS_RECEIVE: Task unblocked. There are messages waiting to be recieved" );

		//Modifying the queue
		receive_queue->size--;
		Message_info *msg_info = &(receive_queue->msg_infos[receive_queue->oldest]);
		if(++(receive_queue->oldest) == MAX_NUM_TASKS) receive_queue->oldest = 0;

		mem_cpy( msg_info->msg, msg, msg_info->msglen );
		*sender_tid = msg_info->sender_tid;

		//Save REPLY information
		
		Reply_info *reply_info = &(receiver_td->reply_infos[msg_info->sender_tid]);

		reply_info->sender_tid = msg_info->sender_tid;
		reply_info->reply = msg_info->reply;
		reply_info->replylen = msg_info->replylen;

		//Changing the state of the sender
		Task_descriptor *sender_td = &(GLOBALS->tasks[msg_info->sender_tid]);
		sender_td->state = REPLY_TASK;
	}
	//If there are NO sends from other tasks to the current task
	else {
		debug( DBG_KERN, "SYS_RECEIVE: Task unblocked. No tasks in the queue." );

		//Save the current receive arguments
		Receive_info *receive_info = &(receiver_td->receive_info);
		receive_info->sender_tid = sender_tid;
		receive_info->msg = msg;
		receive_info->msglen = msglen;

		//BLOCKING THE TASK//////////////////////////////////
		//Change the state of the calling task
		receiver_td->state = RECEIVE_TASK;
		GLOBALS->schedule.tasks_alive--;

		//Remove the task from the READY queue
		Schedule *sched = &(GLOBALS->schedule);
		Task_queue *pqueue = &(sched->priority[receiver_td->priority]);
		dequeue_tqueue(pqueue);

		debug( DBG_KERN, "SYS_RECEIVE: Task blocked. Removed from schedule." );
	}

	return 0;
}

int
sys_reply( int sender_tid, char *reply, int replylen, Task_descriptor *receiver_td,
		Kern_Globals *GLOBALS ) {

	Reply_info *reply_info = &(receiver_td->reply_infos[sender_tid]);

	mem_cpy( reply, reply_info->reply, replylen );

	Task_descriptor *sender_td = &(GLOBALS->tasks[sender_tid]);

	//Rescheduling the task
	sender_td->state = READY_TASK;
	GLOBALS->schedule.tasks_alive++;

	Task_queue *pqueue = &(GLOBALS->schedule.priority[sender_td->priority]);
	enqueue_tqueue( sender_td, pqueue );

	sys_reschedule( receiver_td, GLOBALS );
	return 0;
}

void
sys_unblock_receive( Task_descriptor *receiver_td, Kern_Globals *GLOBALS ) {

	Message_queue *receive_queue = &(receiver_td->receive_queue);

	//The target task was waiting and there are SOME sends
	if( receiver_td->state != RECEIVE_TASK || receive_queue->size == 0 ) {
		debug( DBG_KERN, "sys_unblock_receive: got called for a non-RECEIVE_TASK or a task with no "
			"messages waiting to be received" );
		return;
	}

	//Modifying the queue
	receive_queue->size--;
	Message_info *msg_info = &(receive_queue->msg_infos[receive_queue->oldest]);
	if( ++(receive_queue->oldest) == MAX_NUM_TASKS ) {
		receive_queue->oldest = 0;
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
	GLOBALS->schedule.tasks_alive++;

	//Rescheduling the task
	Task_queue *pqueue = &(GLOBALS->schedule.priority[receiver_td->priority]);
	enqueue_tqueue(receiver_td, pqueue);
}

int
sys_testcall( int a, int b, int c, int d, int e, int f ) {
//int sys_testcall(int a, int b, int c, int d, int e){ //, int f){
//int sys_testcall(int a, int b, int c, int d){

	return a + b + c + d + e + f;
}
