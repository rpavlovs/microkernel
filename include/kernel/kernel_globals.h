#ifndef __KERNEL_GLOBALS__
#define __KERNEL_GLOBALS__

#define SCHED_QUEUE_LENGTH 100
#define SCHED_NUM_PRIORITIES 16
#define SCHED_TID_MAX_VAL 2147483600

#define MAX_NUM_TASKS	100

#define READY_TASK 	0
#define ACTIVE_TASK 	1
#define ZOMBIE_TASK 	2
#define FREE_TASK 	3
#define SEND_TASK 	4
#define RECEIVE_TASK 	5
#define REPLY_TASK 	6

typedef struct Kern_Globals GLOBALS;

typedef struct {
	int sending_tid;
	char *msg;
	int msglen;
	char *reply;
	int replylen;
} Send_args;

typedef struct {
	int *tid;
	char *msg;
	int msglen;
} Receive_args;

typedef struct {
	int sending_tid;
	char *reply;
	int replylen;
} Args_for_reply;

typedef struct {

	Send_args *args[MAX_NUM_TASKS];

	// position of the newest and oldest td pointers in the queue
	int newest, oldest;

	// Number of tasks are in the queue right now
	int size;

} Wait_queue;

void enqueue_wqueue(Send_args *item, Wait_queue *queue);
Send_args *dequeue_wqueue(Wait_queue *queue);

typedef struct {
	int tid;			// Task ID
	int parent_tid;			// Parent task ID
	int state;			// Current task's state
	int priority;			// ROMA, DON'T DELETE IT!!! O_o   Priority of the current task.
	int *sp;			// Stack pointer
	int spsr;			// SPSR - Saved Program Status Register
	int *lr;			// Link register
	int *fp;			// Frame pointer

	Wait_queue *receive_queue;	//Queue for accepting sends
	Receive_args *receive_args;	//Last arguments for receiving
	Args_for_reply args_for_reply[MAX_NUM_TASKS]; //Arguments for REPLY function
	
} Task_descriptor;

// Schedule structures
typedef struct {

	Task_descriptor *td_ptrs[SCHED_QUEUE_LENGTH];

	// position of the newest and oldest td pointers in the queue
	int newest, oldest;

	// Number of tasks are in the queue right now
	int size;

} Task_queue;

void enqueue_tqueue(Task_descriptor *td, Task_queue *q);
Task_descriptor *dequeue_tqueue(Task_queue *q);

typedef struct {
	Task_queue priority[SCHED_NUM_PRIORITIES];
	
	// The latest tid scheduled to run
	int last_issued_tid;

	// The last activated tid
	int last_active_tid;
	
	int tasks_alive;
} Schedule;


typedef struct {
	Wait_queue wqueues[MAX_NUM_TASKS];
	Task_descriptor tasks[MAX_NUM_TASKS];
	Schedule schedule;
} Kern_Globals;

#endif

