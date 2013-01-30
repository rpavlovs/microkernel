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
int RegisterAs( char *name ) {	
	int msg_length;
	char msg[NS_NAME_MAX_LENGTH + 1], reply_buf[1];
	
	for( msg_length = 0; msg_length < NS_NAME_MAX_LENGTH; ++msg_length ) {
		if( name[msg_length] == '\0' ) break;
	}
	
	assert( msg_length != NS_NAME_MAX_LENGTH, "REGISTER_AS: Name is to long" );
	
	msg[0] = NAMESERVER_REGISTER_REQUEST;
	mem_cpy( name, msg + 1, msg_length );

	Send( NAMESERVER_TID, msg, msg_length + 1, reply_buf, 1 );

	return - (int)(reply_buf[0]);
}

// Sends messege to nameserver [reqest_id][firstcharofname]....[\0] 
int WhoIs( char *name ) {
	int msg_length;
	char msg[NS_NAME_MAX_LENGTH + 1], reply_buf[2];
	
	for( msg_length = 0; msg_length < NS_NAME_MAX_LENGTH; ++msg_length ) {
		if( name[msg_length] == '\0' ) break;
	}
	
	assert( msg_length != NS_NAME_MAX_LENGTH, "REGISTER_AS: Name is to long" );
	
	msg[0] = NS_REQUEST_REGISTER_AS;
	mem_cpy( name, msg + 1, msg_length );

	Send( NS_TID, msg, msg_length + 1, reply_buf, 2 );

	if( reply_buf[0] != 0 ) {
		return - (int)(reply_buf[0]);
	}
	return reply_buf[1];
}

int TestCall(int a, int b, int c, int d, int e, int f){
//int TestCall(int a, int b, int c, int d, int e){ //, int f){
//int TestCall(int a, int b, int c, int d){

	asm ( "swi\t%0"	"\n\t" :: "J" (TESTCALL_SYSCALL) );

}















