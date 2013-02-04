#ifndef __NAMESERVER_H__
#define __NAMESERVER_H__

void timeserver();

// Messages



typedef struct {
	int type;
	/* data */
} Msg_timeserver_request;

#define TIME_REQUEST				0
#define DELAY_REQUEST				1
#define DELAY_UNTIL_REQUEST			2

#define TIME_REPLY					1

typedef struct {
	int type;
	int num;
} Msg_timeserver_reply;

#endif
