#ifndef __KERNEL_GLOBALS__
#define __KERNEL_GLOBALS__

#define SCHED_QUEUE_LENGTH 					100
#define SCHED_NUM_PRIORITIES 				21
#define SCHED_TID_MAX_VAL 					2147483600

#define MAX_NUM_TASKS						100
					
#define READY_TASK 							0
#define ACTIVE_TASK 						1
#define ZOMBIE_TASK 						2
#define FREE_TASK 							3
#define SEND_TASK 							4
#define RECEIVE_TASK 						5
#define REPLY_TASK 							6

#define MAX_NUM_ARGUMENTS 					10

#define SEND_ERROR_TID_IMPOSSIBLE			-1
#define SEND_ERROR_TID_HAS_NO_TASK			-2
#define SEND_ERROR_TRANSACTION_FAILED		-3

#define REPLY_ERROR_TID_IMPOSSIBLE			-1
#define REPLY_ERROR_TID_HAS_NO_TASK			-2
#define REPLY_ERROR_NO_ONE_WAITING			-3
#define REPLY_ERROR_SEND_BUFFER_TOO_SHORT	-4


typedef struct Kern_Globals GLOBALS;

typedef struct {
	int sender_tid;
	char *msg;
	int msglen;
	char *reply;
	int replylen;
} Message_info;

typedef struct {
	int *sender_tid;	// needs to be set
	char *msg;
	int msglen;
} Receive_info;

typedef struct {
	int sender_tid;
	char *reply;
	int replylen;
} Reply_info;

typedef struct {
	Message_info msg_infos[MAX_NUM_TASKS];
	int newest, oldest;
	int size;
} Message_queue;

typedef struct {
	int tid;
	int parent_tid;
	int state;
	int priority;
	int *sp;
	int spsr;			// SPSR - Saved Program Status Register
	int *lr;
	int *fp;

	Message_queue receive_queue;			//Queue for recieving messages
	Receive_info receive_info;				//Last arguments for receiving
	Reply_info reply_infos[MAX_NUM_TASKS]; 	//Arguments for REPLY function
	
} Task_descriptor;

// Schedule structures
typedef struct {

	Task_descriptor *td_ptrs[SCHED_QUEUE_LENGTH];

	// position of the newest and oldest td pointers in the queue
	int newest, oldest;

	// Number of tasks are in the queue right now
	int size;

} Task_queue;

typedef struct {
	Task_queue priority[SCHED_NUM_PRIORITIES];
	
	// The latest tid scheduled to run
	int last_issued_tid;

	// The last activated tid
	int last_active_tid;
	
	int tasks_alive;
} Schedule;


typedef struct {
	Task_descriptor tasks[MAX_NUM_TASKS];
	Schedule schedule;
} Kern_Globals;

#endif

