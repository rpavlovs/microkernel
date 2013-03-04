#include "kernelspace.h"

/////////////////////////////////////////////////////////////////////
//
// Message queues
//
/////////////////////////////////////////////////////////////////////
void enqueue_msg_queue(int sender_tid, char *msg, int msglen,
						char *reply, int replylen, Message_queue *mailbox) {

	assert(mailbox->size < MSG_QUEUE_SIZE, "Message queue should not overflow");

	//Iterate the index
	mailbox->size++;
	if(++(mailbox->newest) >= MSG_QUEUE_SIZE) mailbox->newest = 0;

	//Get the message info to modify
	Send_info *msg_info = &(mailbox->msg_infos[mailbox->newest]);
	msg_info->sender_tid = sender_tid;
	msg_info->msg = msg;
	msg_info->msglen = msglen;
	msg_info->reply = reply;
	msg_info->replylen = replylen;
}

void dequeue_msg_queue(Message_queue *mailbox){
	assert(mailbox->size > 0, "Message queue should have items to dequeue");

	if( ++(mailbox->oldest) >= MSG_QUEUE_SIZE ) mailbox->oldest = 0;
	mailbox->size--;
}



/////////////////////////////////////////////////////////////////////
//
// Task queues
//
/////////////////////////////////////////////////////////////////////

/*
void enqueue_tqueue(Task_descriptor *td, Task_queue *q){
	assert(q->size != MAX_NUM_TASKS, "Task queue should not overflow :)");

	//Modifying the queue
	q->size++;
	if(++(q->newest) == MAX_NUM_TASKS) q->newest = 0;
	q->td_ptrs[q->newest] = td;
}

Task_descriptor *dequeue_tqueue(Task_queue *q){
	assert(q->size != 0, "Task queue should have items to dequeue");

	//Modifying the queue
	q->size--;
	Task_descriptor *td = q->td_ptrs[q->oldest];
	if(++(q->oldest) == MAX_NUM_TASKS) q->oldest = 0;
	return td;
}*/
