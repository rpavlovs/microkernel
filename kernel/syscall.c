#include "kernel/syscall.h"
#include "lib/bwio.h"

int Create( int priority, void (*code) ( ) ) {

	Syscall_args args;

	args.priority = priority;
	args.code = code;

	//DEBUGGING
	bwprintf( COM2, "CREATE: before SWI\n\r" );

	asm ( "swi\t%0"	"\n\t" :: "J" (CREATE_SYSCALL) );

	return args.ret;
}

int MyTid( ) {

	Syscall_args args;

	asm ( "swi\t%0"	"\n\t" :: "J" (MYTID_SYSCALL) );

	return args.ret;
}


int MyParentTid( ) {

	Syscall_args args;

	asm ( "swi\t%0"	"\n\t" :: "J" (MYPARENTTID_SYSCALL) );

	return args.ret;
}


void Pass( ) {

	asm ( "swi\t%0"	"\n\t" :: "J" (PASS_SYSCALL) );

}


void Exit( ) {

	asm ( "swi\t%0"	"\n\t" :: "J" (EXIT_SYSCALL) );

}

