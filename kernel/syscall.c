#include "kernel/syscall.h"
#include "lib/bwio.h"



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

int Reply(int tid, char *reply, int replylen){

	asm ( "swi\t%0"	"\n\t" :: "J" (REPLY_SYSCALL) );

}

int TestCall(int a, int b, int c, int d, int e, int f){
//int TestCall(int a, int b, int c, int d, int e){ //, int f){
//int TestCall(int a, int b, int c, int d){

	asm ( "swi\t%0"	"\n\t" :: "J" (TESTCALL_SYSCALL) );

}















