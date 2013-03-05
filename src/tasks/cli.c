#include "userspace.h"

void init_server_list( Servers_tid_list *servers_list ){
	int cmd_server_tid = WhoIs( COMMAND_SERVER_NAME ); 
	servers_list->items[ CMD_SERVER_INDEX ] = cmd_server_tid; 
	bwassert( cmd_server_tid >= 0, "CLI: This task requires the command server to work properly." );
	
	int sw_server_tid = WhoIs( SWITCHES_SERVER_NAME );
	servers_list->items[SWITCHES_SERVER_INDEX] = sw_server_tid; 
	bwassert( cmd_server_tid >= 0, "CLI: This task requires the command server to work properly." );
}

void task_cli() {

	CLI_history history;
	init_cli_history( &history );
	
	// Retrieve the tid of the tasks required for the CLI to work properly. 
	Servers_tid_list servers_list; 
	init_server_list( &servers_list ); 

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
			if( buf.size > 0 ){
				char_queue_pop_back( &buf ); 
				Putstr( COM2, "\b \b" );
			}
			break;
		case CHAR_NEWLINE:
		case CHAR_RETURN:
			// Get and parses the executed command. 
			char_queue_push( '\0', &buf );
			char_queue_peek_str( &buf, cmd, CLI_COMMAND_MAX_LENGTH );
			status = parse_and_exec_cmd( &buf, &servers_list );
			
			// Shows the result of the command in the screen. It also shows the history of previously 
			// executed commands.
			history_push( &history, cmd, status );
			update_cli_view( &history );
			
			// Clear the buffer. 
			init_char_queue( &buf ); 
			break;
		default:
			// TODO: check that new char is sensible
			char_queue_push( c, &buf );
			//printf( COM2, "%c", c );
			Putc( COM2, c ); 
		}
	}
}

// returns the execution status 
int parse_and_exec_cmd( Char_queue *buf, Servers_tid_list *servers_list ) {
	char cmd_name[CLI_COMMAND_MAX_LENGTH];
	char_queue_pop_word( buf, cmd_name, CLI_COMMAND_MAX_LENGTH );
	printf( COM2, "%s\n", buf );
	if( strcmp( cmd_name, "tr" ) == 0 ) {
		char train_id_str[3], speed_str[3];
		char_queue_pop_word( buf, train_id_str, 3 );
		char_queue_pop_word( buf, speed_str, 3 );

		if( train_id_str == '\0' || train_id_str == '\0' )
			return INVALID_NUMBER_OF_ARGUMENTS;

		// NOTE: If the speed is not specified it will default to 0. 
		return exec_tr( atoi(train_id_str), atoi(speed_str), servers_list );
	}
	if( strcmp( cmd_name, "sw" ) == 0 ) {
		char switch_id_str[3], switch_state[2];
		char_queue_pop_word( buf, switch_id_str, 3 );
		char_queue_pop_word( buf, switch_state, 2 );

		if( switch_id_str == '\0' || switch_state == '\0' )
			return INVALID_NUMBER_OF_ARGUMENTS;

		return exec_sw( atoi(switch_id_str), switch_state[0], servers_list );
	}
	if( strcmp( cmd_name, "rv" ) == 0 ) {
		char train_id_str[3];
		char_queue_pop_word( buf, train_id_str, 3 );
		return exec_rv( atoi(train_id_str), servers_list );
	}
	if( strcmp( cmd_name, "q" ) == 0 ) {
		return exec_q( );
	}
	return INVALID_COMMAND_NAME;
}

void send_command( int cmd_type, int element_id, int param, int server_tid ){
	Cmd_request cmd_request;
	cmd_request.type = ADD_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = cmd_type; 
	cmd_request.cmd.element_id = element_id; 
	cmd_request.cmd.param = param; 
	
	Send( server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  ); 
}

int exec_tr( int train_id, int speed, Servers_tid_list *servers_list ) {
	if( !is_train_id(train_id) )
		return INVALID_TRAIN_ID;
	if( speed < 0 || speed > 14 ) 
		return INVALID_SPEED;
	
	// Send message to command server.  
	int cmd_server_tid = servers_list->items[CMD_SERVER_INDEX]; 
	send_command( TRAIN_CMD_TYPE, train_id, speed, cmd_server_tid ); 
	return SUCCESS;
}

int exec_sw( int switch_id, char state, Servers_tid_list *servers_list ) {
	if( !(1 <= switch_id && switch_id <= 18) && !(153 <= switch_id && switch_id <= 156) )
		return INVALID_SWITCH_ID;
	if( !(state == 'C' || state == 'S') )
		return INVALID_SWITCH_STATE;

	// Send message to command server. 
	int sw_server_tid = servers_list->items[SWITCHES_SERVER_INDEX]; 
	send_command( SWITCH_CMD_TYPE, switch_id, (int) state, sw_server_tid ); 
	return SUCCESS;
}

int exec_rv( int train_id, Servers_tid_list *servers_list ) {
	if( !is_train_id(train_id) )
		return INVALID_TRAIN_ID;
	
	// Send message to command server. 
	int cmd_server_tid = servers_list->items[CMD_SERVER_INDEX]; 
	send_command( REVERSE_CMD_TYPE, train_id, CMD_PARAM_NOT_REQUIRED, cmd_server_tid ); 
	return SUCCESS;
}

int exec_q( ) {
	Shutdown(); 
	return SUCCESS;	// This should never execute. 
}

void init_cli_history( CLI_history *h ) {
	h->head = h->inputs;
	h->tail = h->inputs;
	h->size = 0;
}

/*
void history_push( CLI_history *h, char *cmd, int status ) {
	if( h->size >= CLI_HISTORY_LENGTH ) {
		//discard command from the tail to fit a new one
		if( ++(h->tail) >= h->inputs + CLI_HISTORY_LENGTH ){
			h->tail = h->inputs;
		}
	} else {
		h->size++;
	}
	if( ++(h->head) >= h->inputs + CLI_HISTORY_LENGTH ){
		h->head = h->inputs;
	}
	strcpy( cmd, h->head->command );
	h->head->status = status;
}

 */
void history_push( CLI_history *h, char *cmd, int status ) {
	// TODO: Add the history feature. For now we're just using the head of the
	// history queue. 
	strcpy( cmd, h->head->command );
	h->head->status = status;
}


void update_cli_view( CLI_history *h ) {
	int history_row, inputs_poped;
	Input *history_record;
	char str[CLI_PRINT_BUFFER_SIZE], *ptr;

	// TODO
	ptr = str;
	ptr += sprintf( ptr, "\033[?25l" );	// DISABLE THE CURSOR. 
	// ----------------------------------------------------------------------------------------------------------------------------------------
	// History
	// --> This part displays a small list of the recently commands issued, and their result. 
	// ----------------------------------------------------------------------------------------------------------------------------------------
	inputs_poped = 0; 
	history_record = h->head;
	history_row = UI_CLI_HISTORY_TOP_ROW; 
	
	// TODO: 
	// For now just print the last command issued and the result. Later we will have a whole history feature. 
	ptr += sprintf( ptr, "\033[%d;4H\033[K%s", history_row, history_record->command ); 
	ptr += sprintf( ptr, "\033[%d;7H\033[K", history_row + 1 ); 
	
	switch( history_record->status ) {
	case SUCCESS:
		break;
	case INVALID_TRAIN_ID:
		ptr += sprintf( ptr, "%s", "Error: Invalid train id was specified.");
		break;
	case INVALID_SPEED:
		ptr += sprintf( ptr, "%s", "Error: Invalid train speed was specified.");
		break;
	case INVALID_SWITCH_ID:
		ptr += sprintf( ptr, "%s", "Error: Invalid switch id was specified.");
		break;
	case INVALID_SWITCH_STATE:
		ptr += sprintf( ptr, "%s", "Error: Invalid switch state was specified.");
		break;
	case INVALID_COMMAND_NAME:
		ptr += sprintf( ptr, "%s", "Error: Invalid command name.");
		break;
	case INVALID_NUMBER_OF_ARGUMENTS:
		ptr += sprintf( ptr, "%s", "Error: Invalid number of arguments.");
		break;
	default:
		ptr += sprintf( ptr, "%s", "Unknown error.");
	}
	/*
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
		case INVALID_NUMBER_OF_ARGUMENTS:
			ptr += sprintf( ptr, "%s", "Error: Invalid number of arguments.");
			break;
		default:
			ptr += sprintf( ptr, "%-81s", "Unknown error.");
		}
		if( history_record->status != SUCCESS ) --history_row;
		if( --history_record < h->inputs ) history_record += CLI_HISTORY_LENGTH;
	}
	*/
	ptr += sprintf( ptr, "\033[%d;7H", UI_CLI_CMD_LINE_ROW ); // Position the line to the initial line. 
	ptr += sprintf( ptr, "\033[K\0" );	// Erases the rest of the line. 
	ptr += sprintf( ptr, "\033[?25h" );	// Show the cursor again. 
	

	// TODO: user bwassert( ptr - str < CLI_PRINT_BUFFER_SIZE );

	// TODO: make printf send strings in one message to uart_server 
	Putstr( COM2, str );
}


