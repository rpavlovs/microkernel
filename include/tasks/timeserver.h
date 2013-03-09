#ifndef __NAMESERVER_H__
#define __NAMESERVER_H__

#define TIMESERVER_WAKEUP_QUEUE_SIZE	100

// ----------------------------------------------------------------------------------------
// Time Server Structs
// ----------------------------------------------------------------------------------------

typedef struct Wakeup_record Wakeup_record;

struct Wakeup_record {
	int wakeup_time;
	int tid; 
	Wakeup_record *next_to_wakeup;
};

typedef struct {
	Wakeup_record records[TIMESERVER_WAKEUP_QUEUE_SIZE];
	Wakeup_record *first_to_wakeup;
	int size;
} Wakeup_list;

void timeserver();


// ----------------------------------------------------------------------------------------
// Time Server Message Structs
// ----------------------------------------------------------------------------------------

typedef struct {
	int type;
	int num;
} Msg_timeserver_request;

typedef struct {
	int type;
	int num;
} Msg_timeserver_reply;

#define TIME_SERVER_NAME			"timeserver"

#define TIME_REQUEST				0
#define DELAY_REQUEST				1
#define DELAY_UNTIL_REQUEST			2
#define TICK_NOTIFICATION			3

#define TIME_REPLY					2

#define TIMER_CYCLES_PER_TICK		20

#define TIMER_ENABLE_FLAG			0x80 // 10000000
#define TIMER_MODE					0x40 // 1000000

// If this flag is enabled the system will use a 508 kHz clock frequency. 
// If this flag is disabled the system will use a 2 kHz clock frequency. 
#define TIMER_CLKSEL				0x8  // 1000; 


#endif
