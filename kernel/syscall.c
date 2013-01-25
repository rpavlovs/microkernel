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

