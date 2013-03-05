#include "kernelspace.h"

int Create( int priority, void (*code) ( ) ) {
	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (CREATE_SYSCALL) );
}

int MyTid( ) {
	bwdebug( DBG_REQ, "MY_TID: request recieved." );

	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (MYTID_SYSCALL) );
}

int MyParentTid( ) {
	bwdebug( DBG_REQ, "MY_PARENT_TID: request recieved." );

	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (MYPARENTTID_SYSCALL) );
}

void Pass( ) {
	bwdebug( DBG_REQ, "PASS: request recieved." );

	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (PASS_SYSCALL) );
}

void Exit( ) {
	bwdebug( DBG_REQ, "EXIT: request recieved." );

	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (EXIT_SYSCALL) );
}

int Send( int tid, char *msg, int msglen, char *reply, int replylen ) {
	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (SEND_SYSCALL) );
}

int Receive( int *tid, char *msg, int msglen ) {
	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (RECEIVE_SYSCALL) );
}

int Reply( int tid, char *reply, int replylen ) {
	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (REPLY_SYSCALL) );
}

int AwaitEvent( int eventid, int event ) {
	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (AWAIT_EVENT_SYSCALL) );	
}

int Shutdown(){
	asm( "SWI	%[call_id]" "\n\t" :: [call_id] "J" (SHUTDOWN_EVENT_SYSCALL) );
}

////////////////////
// Wrappers
////////////////////
 
int RegisterAs( char *name ) {
	bwdebug( DBG_REQ, "REGISTER_AS: request recieved. Register [%s]", name );

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
	strcpy( name, request.ns_name );

	status = Send( NAMESERVER_TID, (char *) &request, sizeof(request),
			(char *) &reply, sizeof(reply) );

	if( status == SEND_ERROR_TID_IMPOSSIBLE || status == SEND_ERROR_TID_HAS_NO_TASK ) {
		bwdebug( DBG_REQ,
			"WhoIs: *ERROR* can't get rich of nameserver. Invalid task ID." );
		return NS_ERROR_TID_IS_NOT_A_TASK;
	}
	if( status == SEND_ERROR_TRANSACTION_FAILED || status == ERROR_WRONG_MESSAGE_TYPE ) {
		bwdebug( DBG_REQ,
			"WhoIs: *ERROR* communication with nameserver failed." );
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;
	}

	return reply.num;
}

int WhoIs( char *name ) {
	bwdebug( DBG_REQ, "WHOIS: request recieved. Lookup [%s]", name );

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
	strcpy( name, request.ns_name );

	status = Send( NAMESERVER_TID, (char *) &request, sizeof(request),
					(char *) &reply, sizeof(reply) );

	if( status == SEND_ERROR_TID_IMPOSSIBLE || status == SEND_ERROR_TID_HAS_NO_TASK ) {
		bwdebug( DBG_REQ,
			"WhoIs: *ERROR* can't get rich of nameserver." );
		return NS_ERROR_TID_IS_NOT_A_TASK;
	}
	if( status == SEND_ERROR_TRANSACTION_FAILED || status == ERROR_WRONG_MESSAGE_TYPE ) {
		bwdebug( DBG_REQ,
			"WhoIs: *ERROR* communication with nameserver failed." );
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;
	}
	return reply.num;
}

int Time() {
	bwdebug( DBG_REQ, "TIME: request recieved." );
	Msg_timeserver_request request_msg;
	Msg_timeserver_reply reply_msg;
	request_msg.type = TIME_REQUEST;

	int status = Send( WhoIs("timeserver"), (char *) &request_msg, sizeof(request_msg),
						(char *) &reply_msg, sizeof(reply_msg) );

	bwassert( reply_msg.type == TIME_REPLY, "Time: Should get proper reply message" );

	return (status < 0 ? status : reply_msg.num);
}

int Delay( int ticks ) {
	bwdebug( DBG_REQ, "DELAY: request recieved. Wait for %d ticks", ticks );
	Msg_timeserver_request request_msg;
	Msg_timeserver_reply reply_msg;
	request_msg.type = DELAY_REQUEST;
	request_msg.num = ticks;

	return Send( WhoIs("timeserver"), (char *) &request_msg, sizeof(request_msg),
						(char *) &reply_msg, sizeof(reply_msg) );
}

int DelayUntil( int ticks ) {
	bwdebug( DBG_REQ, "DELAY_UNTIL: request recieved. Wait till %d'th "
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

/////////////////////////////////////////////////
//
// IO Wrappers
//
/////////////////////////////////////////////////
int Getc( int channel ) {
	int tid = -1;
	UART_request request;
	UART_reply reply;

	todo_debug( 0x1, 0 );

	if(channel == COM1) {
		todo_debug( 0x2, 0 );
		//Train controller
		tid = WhoIs("uart1_receiver");
		request.type = UART1_RECEIVE_REQUEST;
		todo_debug( 0x3, 0 );
	}
	else if(channel == COM2) {
		todo_debug( 0x4, 0 );
		//Terminal
		tid = WhoIs("uart2_receiver");
		request.type = UART2_RECEIVE_REQUEST;
		todo_debug( 0x5, 0 );
	}
	else{
		todo_debug( 0x6, 0 );
		//COM channel is invalid
		return -3;
	}

	//check the return value of WhoIs
	if(tid < 0) {
		todo_debug( 0x7, 0 );
		return -1;
	}

	//Sending the message to the server
	todo_debug( 0x8, 0 );
	Send(tid, (char *) &request, sizeof(request), (char *) &reply, sizeof(reply));
	todo_debug( 0x9, 0 );

	return (int) reply.ch;
}

int Putc( int channel, char ch ) {
	if( channel != COM1 && channel != COM2 ) return -3; //COM channel is invalid

	int server_tid = WhoIs( channel == COM1 ? "uart1_sender" : "uart2_sender" );
	if( server_tid < 0 ) return -1;

	UART_request req;
	req.type = UART_SEND_REQUEST_PUTC;
	req.ch = ch;

	Send( server_tid, (char *) &req, sizeof(req), 0, 0 );
	return 0;
}

int Putstr( int channel, char *str ) {
	if( channel != COM1 && channel != COM2 ) return -3; //COM channel is invalid

	int server_tid = WhoIs( channel == COM1 ? "uart1_sender" : "uart2_sender" );
	if( server_tid < 0 ) return -1;

	UART_request req;
	req.type = UART_SEND_REQUEST_PUTSTR;
	req.str = str;
	
	Send( server_tid, (char *) &req, sizeof(req), 0, 0 );
	return 0;
}
















