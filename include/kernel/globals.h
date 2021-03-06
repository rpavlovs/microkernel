#ifndef __DBG_KERN_GLOBALS__
#define __DBG_KERN_GLOBALS__

#define SCHED_QUEUE_LENGTH 					100
#define SCHED_NUM_PRIORITIES 				16
#define SCHED_TID_MAX_VAL 					2147483600

#define MAX_NUM_TASKS						100
					
#define READY_TASK 							0
#define ACTIVE_TASK 						1
#define ZOMBIE_TASK 						2
#define FREE_TASK 							3
#define SEND_TASK 							4
#define RECEIVE_BLOCKED 					5
#define REPLY_TASK 							6
#define AWAIT_TASK 							7

#define MAX_NUM_ARGUMENTS 					10

#define SEND_ERROR_TID_IMPOSSIBLE			-1
#define SEND_ERROR_TID_HAS_NO_TASK			-2
#define SEND_ERROR_TRANSACTION_FAILED		-3

#define REPLY_ERROR_TID_IMPOSSIBLE			-1
#define REPLY_ERROR_TID_HAS_NO_TASK			-2
#define REPLY_ERROR_NO_ONE_WAITING			-3
#define REPLY_ERROR_SEND_BUFFER_TOO_SHORT	-4

#define HWI_NUM_EVENTS						10

typedef struct Kern_Globals GLOBALS;

// Send/Receive/Reply structs

typedef struct {
	int sender_tid;
	char *msg;
	int msglen;
	char *reply;
	int replylen;
} Send_info;

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
	Send_info msg_infos[MAX_NUM_TASKS];
	int newest, oldest;
	int size;
} Message_queue;

typedef struct {
	char *eventBuffer;
	int bufferLength;
} Await_event_info;

// Task descriptor

typedef struct {
	int tid;
	int parent_tid;
	int state;
	int priority;
	int *sp;
	int spsr;			// SPSR - Saved Program Status Register
	int *lr;
	int *fp;

	Message_queue mailbox;			//Queue for recieving messages
	Receive_info receive_info;				//Last arguments for receiving
	Reply_info reply_infos[MAX_NUM_TASKS]; 	//Arguments for REPLY function
	int event_char;
} Task_descriptor;

// Scheduler structures

typedef struct {
	Task_descriptor *td_ptrs[SCHED_QUEUE_LENGTH];
	int newest, oldest;
	int size;
} Task_queue;

typedef struct {
	Task_queue queues[SCHED_NUM_PRIORITIES];
	int last_issued_tid;
	int last_active_tid;
	int tasks_alive;
	int tasks_exited;
	unsigned long latest_idle_task_seqence_activated_at;

	Task_descriptor *hwi_watchers[HWI_NUM_EVENTS];
} Scheduler;

// System data

typedef struct {
	unsigned long updated_at;
	unsigned long idle_time_since_update;
	int cpu_load;
	int cpu_utilization;
	unsigned long kernel_loop_cnt;
} Profiling_data;

// Globals

typedef struct {
	Task_descriptor tasks[MAX_NUM_TASKS];
	Scheduler scheduler;
	Profiling_data profdata;
} Kern_Globals;

#endif

