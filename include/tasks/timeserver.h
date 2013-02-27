#ifndef __NAMESERVER_H__
#define __NAMESERVER_H__

void timeserver();

// Messages

typedef struct {
	int type;
	int num;
} Msg_timeserver_request;

typedef struct {
	int type;
	int num;
} Msg_timeserver_reply;

#define TIME_REQUEST				0
#define DELAY_REQUEST				1
#define DELAY_UNTIL_REQUEST			2
#define TICK_NOTIFICATION			3

#define TIME_REPLY					4

#define TIMER_CYCLES_PER_TICK			20

#define TIMER_ENABLE_FLAG			0x80 // 10000000
#define TIMER_MODE					0x40 // 1000000

// If this flag is enabled the system will use a 508 kHz clock frequency. 
// If this flag is disabled the system will use a 2 kHz clock frequency. 
#define TIMER_CLKSEL				0x8  // 1000; 


#endif
