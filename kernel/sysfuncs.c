#include "kernel/kernel_globals.h"
#include "kernel/sysfuncs.h"
#include "kernel/helpers.h"
#include "lib/bwio.h"

int sys_create( int priority, void (*code) ( ), Task_descriptor *td, Kern_Globals *GLOBALS ) {
	
	debug( "sys_create: ENTERED" );

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

	// ASSERT: Verifying the size of the queue
	assert( queue->size < SCHED_QUEUE_LENGTH, "Scheduler queue must not be full" );

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
	sys_reschedule(td, GLOBALS);

	return new_tid;
}

int sys_mytid(Task_descriptor *td, Kern_Globals *GLOBALS )
{
	debug( "sys_mytid: ENTERED" );

	sys_reschedule(td, GLOBALS);
	return td->tid;
}

int sys_myparenttid(Task_descriptor *td, Kern_Globals *GLOBALS )
{
	debug( "sys_myparenttid: ENTERED");

	sys_reschedule(td, GLOBALS);
	return td->parent_tid;
}

void sys_pass(Task_descriptor *td, Kern_Globals *GLOBALS )
{
	debug( "sys_pass: ENTERED" );

	sys_reschedule(td, GLOBALS);
}

void sys_exit(Task_descriptor *td, Kern_Globals *GLOBALS ) 
{
	debug( "sys_exit: ENTERED" );

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
	(pqueue->size)--;

	// Updating the schedule
	sched->tasks_alive--;
}

void sys_reschedule(Task_descriptor *td, Kern_Globals *GLOBALS ){
	debug( "sys_reschedule: ENTERED" );

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

int sys_send(int Tid, char *msg, int msglen, char *reply, int replylen, Task_descriptor *td, Kern_Globals *GLOBALS ){

	//Getting TD of the target task
	Task_descriptor *target_td = &(GLOBALS->tasks[Tid]);

	//Getting the receive queue of the target task
	Wait_queue *receive_queue = target_td->receive_queue;

	//Adding the current send arguments to the target queue
	Send_args args;
	args.sending_tid = td->tid;
	args.msg = msg;
	args.msglen = msglen;
	args.reply = reply;
	args.replylen = replylen;
	
	enqueue_wqueue(&args, receive_queue);

	//Unblocking the target task
	sys_unblock_receive(target_td, GLOBALS);

	//BLOCKING///////////////////////////////////////////////////
	//Change the state of the calling task to SEND_BLOCKED
	td->state = SEND_TASK;
	
	//Remove the task from the READY queue
	int priority = td->priority;
	Schedule *sched = &(GLOBALS->schedule);
	Task_queue *pqueue = &(sched->priority[priority]);
	dequeue_tqueue(pqueue);

	return 0;
}

int sys_receive(int *tid, char *msg, int msglen, Task_descriptor *td, Kern_Globals *GLOBALS ){

	//If there are SOME sends from other tasks to the current task
	if(td->receive_queue->size > 0){
		//Retrieve send arguments from the queue
		Send_args *sargs = dequeue_wqueue(td->receive_queue);

		//Copying the message from sender to receiver
		char *smsg = sargs->msg;
		char *rmsg = msg;

		int i;
		int length;
		if(msglen >= sargs->msglen) length = sargs->msglen;
		else length = msglen;

		for(i=0; i<length; i++){
			*(rmsg++) = *(smsg++);
		}

		//Copying the TID
		*tid = sargs->sending_tid;

		//Save REPLY information
		td->args_for_reply[sargs->sending_tid].sending_tid = sargs->sending_tid;
		td->args_for_reply[sargs->sending_tid].reply = sargs->reply;
		td->args_for_reply[sargs->sending_tid].replylen = sargs->replylen;

		//Changing the state of the sender
		int sending_tid = sargs->sending_tid;
		Task_descriptor *sending_td = &(GLOBALS->tasks[sending_tid]);
		sending_td->state = REPLY_TASK;
	}
	//If there are NO sends from other tasks to the current task
	else{
		//Save the current receive arguments
		Receive_args rargs;
		rargs.tid = tid;
		rargs.msg = msg;
		rargs.msglen = msglen;
		td->receive_args = &rargs;

		//BLOCKING THE TASK//////////////////////////////////
		//Change the state of the calling task
		td->state = RECEIVE_TASK;

		//Remove the task from the READY queue
		int priority = td->priority;
		Schedule *sched = &(GLOBALS->schedule);
		Task_queue *pqueue = &(sched->priority[priority]);
		dequeue_tqueue(pqueue);
	}

	return 0;
}

int sys_reply(int tid, char *reply, int replylen, Task_descriptor *td, Kern_Globals *GLOBALS ){

	//Getting necessary arguments to perform reply
	Args_for_reply args = td->args_for_reply[tid];

	//Copying the reply from receiver to sender
	char *srpl = args->reply; 	//sender's reply buffer
	char *rrpl = reply;		//reciever's reply buffer
	
	int i;
	int length;
	if(replylen >= args->replylen) length = args->replylen;
	else length = replylen;

	for(i=0; i<length; i++){
		*(srpl++) = *(rrpl++);
	}

	//Find task descriptor my tid
	Task_descriptor *sending_td = &(GLOBALS->tasks[tid]);
	sending_td->state = READY_TASK;

	//Rescheduling the task
	int priority = sending_td->priority;
	Schedule *sched = &(GLOBALS->schedule);
	Task_queue *pqueue = &(sched->priority[priority]);
	enqueue_tqueue(sending_td, pqueue);

	return 0;
}

void sys_unblock_receive(Task_descriptor *td, Kern_Globals *GLOBALS ){
	//The target task was waiting and there are SOME sends
	if((td->state == RECEIVE_TASK) && (td->receive_queue->size > 0)){

		//COPYING THE MESSAGE////////////////////////////////
		//Retrieve send arguments from the receive queue
		Send_args *sargs = dequeue_wqueue(td->receive_queue);

		//Retrieve saved receive arguments
		Receive_args *rargs = td->receive_args;

		//Copying the message from sender to receiver
		char *smsg = sargs->msg;
		char *rmsg = rargs->msg;

		int i;
		int length;
		if(rargs->msglen >= sargs->msglen) length = sargs->msglen;
		else length = rargs->msglen;

		for(i=0; i<length; i++){
			*(rmsg++) = *(smsg++);
		}

		//Copying the TID
		*(rargs->tid) = sargs->sending_tid;

		//Save REPLY information
		td->args_for_reply[sargs->sending_tid].sending_tid = sargs->sending_tid;
		td->args_for_reply[sargs->sending_tid].reply = sargs->reply;
		td->args_for_reply[sargs->sending_tid].replylen = sargs->replylen;

		//Changing the state of the sender
		int sending_tid = sargs->sending_tid;
		Task_descriptor *sending_td = &(GLOBALS->tasks[sending_tid]);
		sending_td->state = REPLY_TASK;

		//RESCHEDULING///////////////////////////////////////
		//Unblocking the task
		td->state = READY_TASK;

		//Rescheduling the task
		int priority = td->priority;
		Schedule *sched = &(GLOBALS->schedule);
		Task_queue *pqueue = &(sched->priority[priority]);
		enqueue_tqueue(td, pqueue);
	}
	//The target task was waiting and there are NO sends
	else if((td->state == RECEIVE_TASK) && (td->receive_queue->size == 0)){
		//Do nothing
	}
}

int sys_testcall(int a, int b, int c, int d, int e, int f){
//int sys_testcall(int a, int b, int c, int d, int e){ //, int f){
//int sys_testcall(int a, int b, int c, int d){

	return a + b + c + d + e + f;

}
