#include <userspace.h>

void a3_game() {

	bwsetfifo( COM2, OFF );

	int guess = -1;

	bwprintf( COM2, "\nGuess how many seconds am I going to wait.\n"
		"Hint: it is no more than 10 sec\n\n" );

	bwprintf( COM2, "press any key to continue.\n" );

	bwgetc( COM2 );

	int wait_for = ( Time() % 10 );

	Delay( 100 );
	bwprintf( COM2, "Starting in 5...\n" );
	Delay( 100 );
	bwprintf( COM2, "Starting in 4...\n" );
	Delay( 100 );
	bwprintf( COM2, "Starting in 3...\n" );
	Delay( 100 );
	bwprintf( COM2, "Starting in 2...\n" );
	Delay( 100 );
	bwprintf( COM2, "Starting in 1...\n" );
	Delay( 100 );
	bwprintf( COM2, "Waiting :)\n" );
	Delay( wait_for * 100 );
	bwprintf( COM2, "DONE!\n" );
	bwprintf( COM2, "How much time do you think just passed?\n" );

	guess = -1;
	while( guess != wait_for ) {
		guess = bwgetc( COM2 ) - 48;
		bwprintf( COM2, "%d\n", guess );
		if( guess > 20 || guess < 0 ) {
			bwprintf( COM2, "The answer is between 0 and 10. Try again.\n" );
		} else if( guess < wait_for ) {
			bwprintf( COM2, "The answer greater. Try again.\n" );
		} else if( guess > wait_for ){
			bwprintf( COM2, "The answer smaller. Try again.\n" );
		} else {
			bwprintf( COM2, "Good job. It was %d sec.\n", wait_for );
			bwprintf( COM2, "Bye.\n");
			Exit();
		}
	}

}
