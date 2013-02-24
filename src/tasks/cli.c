#include "userspace.h"

void update_cli( const CLI_history history ) {

}

char *getCommand() {
	char next_char;
	while( next_char = Getc(COM2) != "\n" ) {

	}
}

void task_cli() {
	CLI_history history;
	history.length = 0;

	FOREVER {
		getCommand();
	}

}
