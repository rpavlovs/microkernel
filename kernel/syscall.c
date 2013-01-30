#include "kernel/syscall.h"
#include "kernel/helpers.h"
#include "lib/bwio.h"
#include "kernel/kernel_globals.h"


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
//
// Sends messege to nameserver [reqest_id][trearid][firstcharofname]....[\0]
// 
// Note: name should be a null-terminated string with no more than 10 charachters.
int RegisterAs( char *name ) {	
	int msg_length, status;
	char msg[NS_NAME_MAX_LENGTH + 1], reply_buf[1];
	
	for( msg_length = 0; msg_length < NS_NAME_MAX_LENGTH; ++msg_length ) {
		if( name[msg_length] == '\0' ) break;
	}
	
	if( msg_length >= NS_NAME_MAX_LENGTH ) return NS_ERROR_NOT_LEGAL_NAME;

	msg[0] = NS_REQUEST_REGISTER_AS;
	my_strcpy( name, msg + 1 );

	// bwprintf( COM2, "DEBUG: RegisterAs: received_name: %s msg: %s msg_length: %d\n", 
	// 	name, msg + 1, msg_length);

	status = Send( NS_TID, msg, msg_length + 2, reply_buf, 1 );

	if( status == SEND_ERROR_TID_IMPOSSIBLE || status == SEND_ERROR_TID_HAS_NO_TASK )
		return NS_ERROR_TID_IS_NOT_A_TASK;

	if( status == SEND_ERROR_TRANSACTION_FAILED )
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;

	return reply_buf[0];
}

// Sends messege to nameserver [reqest_id][firstcharofname]....[\0] 
int WhoIs( char *name ) {
	int msg_length, status;
	char msg[NS_NAME_MAX_LENGTH + 1], reply_buf[2];
	
	for( msg_length = 0; msg_length < NS_NAME_MAX_LENGTH; ++msg_length ) {
		if( name[msg_length] == '\0' ) break;
	}
	
	if( msg_length >= NS_NAME_MAX_LENGTH ) return NS_ERROR_NOT_LEGAL_NAME;
	
	msg[0] = NS_REQUEST_WHO_IS;
	my_strcpy( name, msg + 1 );

	status = Send( NS_TID, msg, msg_length + 2, reply_buf, 2 );

	if( status == SEND_ERROR_TID_IMPOSSIBLE || status == SEND_ERROR_TID_HAS_NO_TASK )
	return NS_ERROR_TID_IS_NOT_A_TASK;

	if( status == SEND_ERROR_TRANSACTION_FAILED )
		return NS_ERROR_TID_IS_NOT_A_NAMESERVER;

	if( ( -reply_buf[0] ) == NS_ERROR_TASK_NOT_FOUND )
		return NS_ERROR_TASK_NOT_FOUND;

	return reply_buf[1];
}

int TestCall(int a, int b, int c, int d, int e, int f) {
//int TestCall(int a, int b, int c, int d, int e){ //, int f){
//int TestCall(int a, int b, int c, int d){

	asm ( "swi\t%0"	"\n\t" :: "J" (TESTCALL_SYSCALL) );

}















