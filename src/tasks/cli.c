#include "userspace.h"

int parse_and_exec_cmd( Char_queue *buf );
void init_cli_history( CLI_history *h );
void update_cli_view( CLI_history *h );
void history_push( CLI_history *h, char * cmd, int status );
int exec_tr( int train_id, int speed );
int exec_sw( int switch_id, char state );
int exec_rv( int train_id );
int exec_q( );

void task_cli() {

	CLI_history history;
	init_cli_history( &history );

	Char_queue buf;
	init_char_queue( &buf );
	char cmd[CLI_COMMAND_MAX_LENGTH];
	char c;
	int status;

	FOREVER {
		c = Getc(COM2);
		switch( c ) {
		case CHAR_ARROW_UP:
			// TODO: make history work
			break;
		case CHAR_ARROW_DOWN:
			// TODO: make history work
			break;
		case CHAR_BACKSPACE:
			// TODO: make backspace work
			break;
		case CHAR_NEWLINE:
		case CHAR_RETURN:
			enqueue_char_queue( '\0', &buf );
			char_queue_peek_str( &buf, cmd, CLI_COMMAND_MAX_LENGTH );
			status = parse_and_exec_cmd( &buf );
			history_push( &history, cmd, status );
			update_cli_view( &history );
			break;
		default:
			// TODO: check that new char is sensible
			enqueue_char_queue( c, &buf );
			printf( COM2, "%c", c );
		}
	}
}

// returns the execution status 
int parse_and_exec_cmd( Char_queue *buf ) {
	char cmd_name[CLI_COMMAND_MAX_LENGTH];
	char_queue_pop_word( buf, cmd_name, CLI_COMMAND_MAX_LENGTH );
	
	if( strcmp( cmd_name, "tr" ) == 0 ) {
		char train_id_str[2], speed_str[2];
		char_queue_pop_word( buf, train_id_str, 2 );
		char_queue_pop_word( buf, speed_str, 2 );
		return exec_tr( atoi(train_id_str), atoi(speed_str) );
	}
	if( strcmp( cmd_name, "sw" ) == 0 ) {
		char switch_id_str[3];
		char_queue_pop_word( buf, switch_id_str, 3 );
		return exec_sw( atoi(switch_id_str), dequeue_char_queue(buf) );
	}
	if( strcmp( cmd_name, "rv" ) == 0 ) {
		char train_id_str[2];
		char_queue_pop_word( buf, train_id_str, 2 );
		return exec_rv( atoi(train_id_str) );
	}
	if( strcmp( cmd_name, "q" ) == 0 ) {
		return exec_q( );
	}
	return INVALID_COMMAND_NAME;
}

int exec_tr( int train_id, int speed ) {
	if( !is_train_id(train_id) )
		return INVALID_TRAIN_ID;
	if( speed < 0 || speed > 14 ) 
		return INVALID_SPEED;
	
	// TODO: send message to track control server
	return SUCCESS;
}

int exec_sw( int switch_id, char state ) {
	if( !(1 <= switch_id && switch_id <= 18) && !(153 <= switch_id && switch_id <= 156) )
		return INVALID_SWITCH_ID;
	if( !(state == 'C' || state == 'S') )
		return INVALID_SWITCH_STATE;

	// TODO: send message to track control server
	
	return SUCCESS;
}

int exec_rv( int train_id ) {
	if( !is_train_id(train_id) )
		return INVALID_TRAIN_ID;
	
	// TODO: send message to track control server
	return SUCCESS;
}

int exec_q( ) {
	// TODO: request system exit
	Exit();
	return SUCCESS;
}

void init_cli_history( CLI_history *h ) {
	h->head = h->inputs;
	h->tail = h->inputs;
	h->size = 0;
}

void history_push( CLI_history *h, char *cmd, int status ) {
	if( h->size >= CLI_HISTORY_LENGTH ) {
		//discard command from the tail to fit a new one
		if( ++(h->tail) >= h->inputs + CLI_HISTORY_LENGTH )
			h->tail = h->inputs;
	} else {
		h->size++;
	}
	if( ++(h->head) >= h->inputs + CLI_HISTORY_LENGTH )
		h->head = h->inputs;
	strcpy( cmd, h->head->command );
	h->head->status = status;
}


void update_cli_view( CLI_history *h ) {
	int history_row, inputs_poped;
	Input *history_record;
	char str[CLI_PRINT_BUFFER_SIZE], *ptr;

	ptr = str + sprintf( str, "\033[?25l\033[s" );
	
	inputs_poped = 0;
	history_record = h->head;
	history_row = UI_CLI_HISTORY_BOTTOM_ROW;
	while( history_row >= UI_CLI_HISTORY_TOP_ROW && inputs_poped++ < h->size ) {	
		
		ptr += sprintf( ptr, "\033[%d;4H%-81s", history_row, history_record->command );
		
		if( --history_row < UI_CLI_HISTORY_TOP_ROW ) break;

		ptr += sprintf( ptr, "\033[%d;4H", history_row );
		switch( history_record->status ) {
		case SUCCESS:
			break;
		case INVALID_TRAIN_ID:
			ptr += sprintf( ptr, "%-81s", "Error: Invalid train id was specified.");
			break;
		case INVALID_SPEED:
			ptr += sprintf( ptr, "%-81s", "Error: Invalid train speed was specified.");
			break;
		case INVALID_SWITCH_ID:
			ptr += sprintf( ptr, "%-81s", "Error: Invalid switch id was specified.");
			break;
		case INVALID_SWITCH_STATE:
			ptr += sprintf( ptr, "%-81s", "Error: Invalid switch state was specified.");
			break;
		case INVALID_COMMAND_NAME:
			ptr += sprintf( ptr, "%-81s", "Error: Invalid command name.");
			break;
		default:
			ptr += sprintf( ptr, "%-81s", "Unknown error.");
		}
		if( history_record->status != SUCCESS ) --history_row;
		if( --history_record < h->inputs ) history_record += CLI_HISTORY_LENGTH;
	}
	ptr += sprintf( ptr, "\033[u\033?25h" );
	ptr += sprintf( ptr, "\033[%d;7H%-81s\033[%d;7H",
		UI_CLI_CMD_LINE_ROW, " ", UI_CLI_CMD_LINE_ROW );

	// TODO: user assert ( ptr - str < CLI_PRINT_BUFFER_SIZE );

	// TODO: make printf send strings in one message to uart_server 
	printf( COM2, str );
}


