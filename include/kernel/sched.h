
#ifndef ___SCHED___
#define ___SCHED___

#define SCHED_QUEUE_MAX_LENGTH 100
#define SCHED_NUM_PRIORITIES 16
#define SCHED_TID_MAX_VAL 2147483600

typedef struct {
	void (*code) ( );
	int tid;
	int registers[16];
} Task;

typedef struct {
	Task queue[SCHED_QUEUE_MAX_LENGTH];
	//what is the oldest element of the queue
	Task *oldest;
	//what is the youngest element of the queue
	Task *newest;
	//how many tasks are in the queue right now
	int size;
} Task_queue;

typedef struct {
	Task_queue *priority[SCHED_NUM_PRIORITIES];
	//what is the latest tid scheduled to run
	int latest_tid;
} Schedule; 

void init_schedule( Schedule * );

int activate( int );

int schedule( Schedule * );

int getNextRequest( Schedule * );

#endif
