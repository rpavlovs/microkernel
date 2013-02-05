#include "kernelspace.h"

int Create( int priority, void (*code) ( ) ) {
	debug( DBG_CURR_LVL, DBG_REQ, "CREATE: request recieved. Priority %d, address %d",
			priority, (int) code );

	int ret;
	asm(
		"SWI	%[call_id]"									"\n\t"
		"MOV	%[ret], r0" 								"\n\t"
		: [ret] "=r"	(ret)
		: [call_id] "J" (CREATE_SYSCALL)
	);
	return ret;  
}

int MyTid( ) {
	debug( DBG_CURR_LVL, DBG_REQ, "MY_TID: request recieved." );

	int ret;
	asm(
		"SWI	%[call_id]"									"\n\t"
		"MOV	%[ret], r0" 								"\n\t"
		: [ret] "=r"	(ret)
		: [call_id] "J" (MYTID_SYSCALL)
	);
	return ret;  
}

int MyParentTid( ) {
	debug( DBG_CURR_LVL, DBG_REQ, "MY_PARENT_TID: request recieved." );

	int ret;
	asm(
		"SWI	%[call_id]"									"\n\t"
		"MOV	%[ret], r0" 								"\n\t"
		: [ret] "=r"	(ret)
		: [call_id] "J" (MYPARENTTID_SYSCALL)
	);
	return ret;  
}

void Pass( ) {
	debug( DBG_CURR_LVL, DBG_REQ, "PASS: request recieved." );

	asm( "SWI	%0"	"\n\t" :: "J" (PASS_SYSCALL) );
}

void Exit( ) {
	debug( DBG_CURR_LVL, DBG_REQ, "EXIT: request recieved." );

	asm( "SWI	%0"	"\n\t" :: "J" (EXIT_SYSCALL) );
}

int Send( int tid, char *msg, int msglen, char *reply, int replylen ) {
	debug( DBG_CURR_LVL, DBG_REQ, "SEND: request recieved. Recipient tid is %d", tid );
	int ret;
	asm(
		"SWI	%[call_id]"									"\n\t"
		"MOV	%[ret], r0" 								"\n\t"
		: [ret] "=r"	(ret)
		: [call_id] "J" (SEND_SYSCALL)
	);
	return ret;
}

int Receive( int *tid, char *msg, int msglen ) {
	debug( DBG_CURR_LVL, DBG_REQ, "RECEIVE: request recieved." );
	int ret;
	asm(
		"SWI	%[call_id]"									"\n\t"
		"MOV	%[ret], r0" 								"\n\t"
		: [ret] "=r"	(ret)
		: [call_id] "J" (RECEIVE_SYSCALL)
	);
	return ret;
}

int Reply( int tid, char *reply, int replylen ) {
	debug( DBG_CURR_LVL, DBG_REQ, "REPLY: request recieved. Recipient tid is %d", tid );

	int ret;
	asm(
		"SWI	%[call_id]"									"\n\t"
		"MOV	%[ret], r0" 								"\n\t"
		: [ret] "=r"	(ret)
		: [call_id] "J" (REPLY_SYSCALL)
	);
	return ret;
}

////////////////////
// Wrappers
////////////////////
 
int RegisterAs( char *name ) {
	debug( DBG_CURR_LVL, DBG_REQ, "REGISTER_AS: request recieved. Register [%s]", name );

	Nameserver_request request;
	Nameserver_reply reply;

	int msg_length, status;
	
	//Iterate until name is terminated
	for( msg_length = 0; msg_length < NAMESERVER_RECORD_NAME_MAX_LENGTH; ++msg_length ) {
		if( name[msg_length] == '\0' ) break;
	}
	
	//Name length is exceeded
	if( msg_length >= NAMESERVER_RECORD_NAME_MAX_LENGTH ) return NS_ERROR_NOT_LEGAL_NAME;

	request.type = NAMESERVER_REGISTER_AS_REQUEST;
	my_strcpy( name, request.ns_name );

	status = Send( NAMESERVER_TID, (char *) &request, sizeof(request),
			(char *) &reply, sizeof(reply) );

	if( status == SEND_ERROR_TID_IMPOSSIBLE || status == SEND_ERROR_TID_HAS_NO_TASK ) {
		debug( DBG_CURR_LVL, DBG_REQ,
			"WhoIs: *ERROR* can't get rich of nameserver. Invalid task ID." );
		return NS_ERROR_TID_IS_NOT_A_TASK;
	}
	if( status == SEND_ERROR_TRANSACTION_FAILED || status == ERROR_WRONG_MESSAGE_TYPE ) {
		debug( DBG_CURR_LVL, DBG_REQ,
			"WhoIs: *ERROR* communication with nameserver failed." );
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;
	}

	return reply.num;
}

int WhoIs( char *name ) {
	debug( DBG_CURR_LVL, DBG_REQ, "WHOIS: request recieved. Lookup [%s]", name );

	Nameserver_request request;
	Nameserver_reply reply;

	int msg_length, status;
	
	//Iterate until name is terminated
	for( msg_length = 0; msg_length < NAMESERVER_RECORD_NAME_MAX_LENGTH; ++msg_length ) {
		if( name[msg_length] == '\0' ) break;
	}
	
	//Name length is exceeded
	if( msg_length >= NAMESERVER_RECORD_NAME_MAX_LENGTH ) return NS_ERROR_NOT_LEGAL_NAME;
	
	request.type = NAMESERVER_WHO_IS_REQUEST;
	my_strcpy( name, request.ns_name );

	status = Send( NAMESERVER_TID, (char *) &request, sizeof(request),
					(char *) &reply, sizeof(reply) );

	if( status == SEND_ERROR_TID_IMPOSSIBLE || status == SEND_ERROR_TID_HAS_NO_TASK ) {
		debug( DBG_CURR_LVL, DBG_REQ,
			"WhoIs: *ERROR* can't get rich of nameserver." );
		return NS_ERROR_TID_IS_NOT_A_TASK;
	}
	if( status == SEND_ERROR_TRANSACTION_FAILED || status == ERROR_WRONG_MESSAGE_TYPE ) {
		debug( DBG_CURR_LVL, DBG_REQ,
			"WhoIs: *ERROR* communication with nameserver failed." );
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;
	}
	return reply.num;
}

int Time() {
	debug( DBG_CURR_LVL, DBG_REQ, "TIME: request recieved." );
	Msg_timeserver_request request_msg;
	Msg_timeserver_reply reply_msg;
	request_msg.type = TIME_REQUEST;

	int status = Send( WhoIs("timeserver"), (char *) &request_msg, sizeof(request_msg),
						(char *) &reply_msg, sizeof(reply_msg) );

	assert( reply_msg.type == TIME_REPLY, "Time: Should get proper reply message" );

	return (status < 0 ? status : reply_msg.num);
}

int Delay( int ticks ) {
	debug( DBG_CURR_LVL, DBG_REQ, "DELAY: request recieved. Waiting for %d ticks", ticks );
	Msg_timeserver_request request_msg;
	Msg_timeserver_reply reply_msg;
	request_msg.type = DELAY_REQUEST;
	request_msg.num = ticks;

	return Send( WhoIs("timeserver"), (char *) &request_msg, sizeof(request_msg),
						(char *) &reply_msg, sizeof(reply_msg) );
}

int DelayUntil( int ticks ) {
	debug( DBG_CURR_LVL, DBG_REQ, "DELAY_UNTIL: request recieved. Waiting till %d'th "
		"tick from start", ticks );
	Msg_timeserver_request request_msg;
	Msg_timeserver_reply reply_msg;
	request_msg.type = DELAY_UNTIL_REQUEST;
	request_msg.num = ticks;

	return Send( WhoIs("timeserver"), (char *) &request_msg, sizeof(request_msg),
						(char *) &reply_msg, sizeof(reply_msg) );
}

int TestCall(int a, int b, int c, int d, int e, int f) {
//int TestCall(int a, int b, int c, int d, int e){ //, int f){
//int TestCall(int a, int b, int c, int d){
	int ret;
	asm(
		"SWI	%[call_id]"									"\n\t"
		"MOV	%[ret], r0" 								"\n\t"
		: [ret] "=r"	(ret)
		: [call_id] "J" (TESTCALL_SYSCALL)
	);
	return ret; 
}
