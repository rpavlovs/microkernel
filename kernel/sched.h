
#define SCHED_QUEUE_MAX_LENGTH 100
#define SCHED_NUM_PRIORITIES 16
#define SCHED_TID_MAX_VAL 2147483600

typedef struct {
	void (*code) ( );
	int tid;
	int registers[16];
} Process;

typedef struct {
	Process buffer[SCHED_QUEUE_MAX_LENGTH];
	Process *oldest;
	Process *newest;
	int size;
} Process_queue;

typedef struct {
	Process_queue *priority[SCHED_NUM_PRIORITIES];
	int latest_tid;
} Schedule; 



int activate( int );

int schedule( int );

int Create( int, void (*) ( ) );

int getNextRequest( );

