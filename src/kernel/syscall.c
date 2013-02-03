#include "kernelspace.h"

int Create( int priority, void (*code) ( ) ) {	
	asm ( "swi\t%0"	"\n\t" :: "J" (CREATE_SYSCALL) );
}

int MyTid( ) {
	asm ( "swi\t%0"	"\n\t" :: "J" (MYTID_SYSCALL) );
}

int MyParentTid( ) {
	asm ( "swi\t%0"	"\n\t" :: "J" (MYPARENTTID_SYSCALL) );
}

void Pass( ) {
	asm ( "swi\t%0"	"\n\t" :: "J" (PASS_SYSCALL) );
}

void Exit( ) {
	asm ( "swi\t%0"	"\n\t" :: "J" (EXIT_SYSCALL) );
}

int Send(int Tid, char *msg, int msglen, char *reply, int replylen){
	asm ( "swi\t%0"	"\n\t" :: "J" (SEND_SYSCALL) );
}

int Receive(int *tid, char *msg, int msglen){
	asm ( "swi\t%0"	"\n\t" :: "J" (RECEIVE_SYSCALL) );
}

int Reply( int tid, char *reply, int replylen ){
	asm ( "swi\t%0"	"\n\t" :: "J" (REPLY_SYSCALL) );
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
