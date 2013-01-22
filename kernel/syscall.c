#define CREATE_SYSCALL 0
#define MYTID_SYSCALL 1
#define MYPARENTTID_SYSCALL 2
#define PASS_SYSCALL 3
#define EXIT_SYSCALL 4

typedef struct {


	// Additional args for create syscall
	int priority;					// fp - 24
	void (*code) ( );			// fp - 20
 
	// Place to put syscall return value (let's hope it's always integer)
	int ret; 							// fp - 16     		

} Syscall_args;


int Create( int priority, void (*code) ( ) ) {

	Syscall_args args;

	args.priority = priority;
	args.code = code;

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
