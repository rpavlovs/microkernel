#include "kernel/syscall.h"
#include "lib/bwio.h"

//extern int Create( int priority, void ( *code ) () ); 

int Create( int priority, void (*code) ( ) ) {
	//DEBUGGING
	//bwprintf( COM2, "Create (system call): ENTERED\n\r");

	//Syscall_args args;

	//args.priority = priority;
	//args.code = code;

	//DEBUGGING
	//bwprintf( COM2, "CREATE: before SWI\n\r" );

	__asm__(
		
		//"MOV	r0, #9\n\t"
		"swi	0"																"\n\t"
	); 	
	//asm ( "swi\t%0"	"\n\t" :: "J" (CREATE_SYSCALL) );

	//return 0;
}

int MyTid( ) {
	//DEBUGGING
	//bwprintf( COM2, "MyTid (system call): ENTERED\n\r");

	//Syscall_args args;

	//asm ( "swi\t%0"	"\n\t" :: "J" (MYTID_SYSCALL) );

	__asm__(
		
		"swi 1"																	"\n\t"
		
		);
	//return args.ret;
	//return 0;
}


int MyParentTid( ) {

	//Syscall_args args;

	asm ( "swi\t%0"	"\n\t" :: "J" (MYPARENTTID_SYSCALL) );

	return 0; //args.ret;
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


















