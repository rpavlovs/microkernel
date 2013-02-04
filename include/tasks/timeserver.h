#ifndef __NAMESERVER_H__
#define __NAMESERVER_H__

void timeserver();

// Messages



typedef struct {
	int type;
	/* data */
} Msg_timeserver_request;

#define TIME_REQUEST_MESSAGE	0

#define TIME_REPLY_MESSAGE		1

typedef struct {
	int type;
	int num;
} Msg_timeserver_reply;

#endif
