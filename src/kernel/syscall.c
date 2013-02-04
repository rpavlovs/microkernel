#include "kernelspace.h"

void swi_jump( Interrupt_info *request_info, const unsigned int request_id ) {
	asm( "MOV	r9 %0"												"\n\t"
		:: "r" (request_info) );
	asm( "SWI	%0"													"\n\t"
		:: "J" (request_id) );
}

int Create( const unsigned int priority, void (const *code) ( ) ) {
	debug( DBG_CURR_LVL, DBG_SYS, "CREATE: Interrupt issued. [priority: %d]", priority );
	Request_info req;
	req.type = CREATE_SYSCALL;
	req.args[0] = (unsigned int) priority;
	req.args[1] = (unsigned int) code;
	swi_jump( &req );
	return (int) req.ret;
}

int MyTid() {
	debug( DBG_CURR_LVL, DBG_SYS, "MYTID: Interrupt issued." );
	Request_info req;
	req.type = MYTID_SYSCALL;
	swi_jump( &req );
	return req.ret;
}

int MyParentTid() {
	debug( DBG_CURR_LVL, DBG_SYS, "MYPARENTTID: Interrupt issued." );
	Request_info req;
	req.type = MYPARENTTID_SYSCALL;
	swi_jump( &req );
	return req.ret;
}

void Pass() {
	debug( DBG_CURR_LVL, DBG_SYS, "PASS: Interrupt issued." );
	Request_info req;
	req.type = PASS_SYSCALL;
	swi_jump( &req );
}

void Exit() {
	debug( DBG_CURR_LVL, DBG_SYS, "EXIT: Interrupt issued." );
	Request_info req;
	req.type = EXIT_SYSCALL;
	swi_jump( &req );
}

int Send( int recipient_tid, char *msg, int msglen, char *reply, int replylen ) {
	debug( DBG_CURR_LVL, DBG_SYS,
		"SEND: Interrupt issued. [recipient_tid: %d]", recipient_tid );
	Request_info req;
	req.type = SEND_SYSCALL;
	ret.args[0] = (unsigned int) recipient_tid;
	ret.args[1] = (unsigned int) msg;
	ret.args[2] = (unsigned int) msglen;
	ret.args[3] = (unsigned int) reply;
	ret.args[4] = (unsigned int) replylen;
	swi_jump( &req );
	return (int) req.ret;
}

int Receive( int *sender_tid, char *msg, int msglen ) {
	debug( DBG_CURR_LVL, DBG_SYS, "RECEIVE: Interrupt issued." );
	Request_info req;
	req.type = RECEIVE_SYSCALL;
	ret.args[0] = (unsigned int) sender_tid;
	ret.args[1] = (unsigned int) msg;
	ret.args[2] = (unsigned int) msglen;
	swi_jump( &req );
	return (int) req.ret;
}

int Reply( int sender_tid, char *reply, int replylen ) {
	debug( DBG_CURR_LVL, DBG_SYS,
		"REPLY: Interrupt issued. [sender_tid: %d]", sender_tid );
	Request_info req;
	req.type = REPLY_SYSCALL;
	ret.args[0] = (unsigned int) sender_tid;
	ret.args[1] = (unsigned int) reply;
	ret.args[2] = (unsigned int) replylen;
	swi_jump( &req );
	return req.ret;
}

////////////////////
// Wrappers
////////////////////
 
int RegisterAs( char *name ) {

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
		debug( DBG_CURR_LVL, DBG_KERN,
			"WhoIs: *ERROR* can't get rich of nameserver. Invalid task ID." );
		return NS_ERROR_TID_IS_NOT_A_TASK;
	}
	if( status == SEND_ERROR_TRANSACTION_FAILED || status == ERROR_WRONG_MESSAGE_TYPE ) {
		debug( DBG_CURR_LVL, DBG_KERN,
			"WhoIs: *ERROR* communication with nameserver failed." );
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;
	}

	return reply.num;
}

int WhoIs( char *name ) {
	debug( DBG_CURR_LVL, DBG_KERN, "WhoIs: request for [%s] recieved.", *name );

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
		debug( DBG_CURR_LVL, DBG_KERN,
			"WhoIs: *ERROR* can't get rich of nameserver." );
		return NS_ERROR_TID_IS_NOT_A_TASK;
	}
	if( status == SEND_ERROR_TRANSACTION_FAILED || status == ERROR_WRONG_MESSAGE_TYPE ) {
		debug( DBG_CURR_LVL, DBG_KERN,
			"WhoIs: *ERROR* communication with nameserver failed." );
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;
	}
	return reply.num;
}

int TestCall(int a, int b, int c, int d, int e, int f) {
//int TestCall(int a, int b, int c, int d, int e){ //, int f){
//int TestCall(int a, int b, int c, int d){
	asm ( "swi\t%0"	"\n\t" :: "J" (TESTCALL_SYSCALL) );
}
