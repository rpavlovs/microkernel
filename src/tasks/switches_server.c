#include <userspace.h>

/* -----------------------------------------------------------------------------------------------------------------------------------------------
 * Switches Server
 *  This server keeps track of the switches position. It prints the current state and redirects the command to the
 *  command server. 
 * NOTE: This server needs to be created after the command server, since the initialization of this task requires
 * initializing the switches. 
 * ----------------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * get_switch_id
 *   This method gets the id of the switch based on its index. The id's are as follow: 
 *	- Low IDs: 1-18
 *	- High IDs: 153-156
 *   NOTE: The sw_index starts at 0 and ends at 21. 
 */
int get_switch_id( int sw_index ){
	// Invalid index? 
	if ( sw_index < 0 || sw_index >= NUM_SWITCHES )
		return -1; 
	
	// Calculate the id based on the index. 
	int sw_id; 
	if ( sw_index < LAST_LOW_SWITCH_INDEX )
		sw_id = sw_index + 1; 
	else
		sw_id = sw_index - LAST_LOW_SWITCH_INDEX + INIT_HIGH_SWITCH_INDEX; 
	
	return sw_id; 
}

int get_switch_index( int sw_id ){
	// Invalid id? 
	if ( (sw_id > LAST_LOW_SWITCH_INDEX && sw_id < INIT_HIGH_SWITCH_INDEX) || 
			sw_id < 1 || sw_id > LAST_HIGH_SWITCH_INDEX )
		return -1; 
	
	int sw_index; 
	if ( sw_id <= LAST_LOW_SWITCH_INDEX )
		sw_index = sw_id - 1;
	else
		sw_index = sw_id - INIT_HIGH_SWITCH_INDEX + LAST_LOW_SWITCH_INDEX;
	
	return sw_index; 
}

int change_switch_position( Cmd_request cmd_request, Switches_list *sw_list, int cmd_server_tid, char *str_buff ){	
	bwdebug( DBG_USR, "CHANGE_SWITCH_POSITION: enters" );
	// Send the command to the cmd_server.
	Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  ); 
	
	// Update the switch position in the list. 
	int sw_index = get_switch_index( cmd_request.cmd.element_id ); 
	char pos = ( char ) cmd_request.cmd.param; 
	sw_list->items[sw_index].position = pos;
	
	// Update the cursor position in the "screen buffer"
	int row, col, size; 
	row = Switches_screen_pos[sw_index][0];
	col = Switches_screen_pos[sw_index][1];  
	
	// Add the new switch position to the "screen buffer"
	size = hideCursor( str_buff );						// Store the current cursor pos.  
	size += cursorPositioning( ( str_buff + size ), row, col );	// Position the cursor to the right position. 
	size += sprintf( ( str_buff + size ), "%c ", pos );			// Show the new cursor position. 
	size += restoreCursor( str_buff + size );				// Restore the cursor to its original pos. 
	
	bwdebug( DBG_USR, "CHANGE_SWITCH_POSITION: exit" );
	return size; 
}

void init_switches( Switches_list *sw_list, int cmd_server_tid, char *buff ){
	bwdebug( DBG_USR, "INIT_SWITCHES: enters" );
	int i, sw_id; 
	char *str_buff = buff; 
	Cmd_request cmd_request; 
	cmd_request.type = ADD_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = SWITCH_CMD_TYPE; 
	cmd_request.cmd.param = SWITCH_STRAIGHT_POS; 
	
	for( i = 0; i < NUM_SWITCHES; i++ ){
		// Get the switch ID from its index
		sw_id = get_switch_id( i ); 
		sw_list->items[ i ].switch_id = sw_id; 

		// Send the command to update the position physically
		cmd_request.cmd.element_id = sw_id; 
		change_switch_position( cmd_request, sw_list, cmd_server_tid, str_buff ); 
		Putstr( COM2, str_buff );
	}
	bwdebug( DBG_USR, "INIT_SWITCHES: done" );
}

void switchserver(){
	// Initialization
	bwdebug( DBG_USR, "SWITCHES SERVER: enters" );
	RegisterAs( SWITCHES_SERVER_NAME );

	int sender_tid; 
	char str_buff[ 1000 ]; 
	char *str_ptr = str_buff; 
	Cmd_request cmd_request;
	
	// The cmd server is required for the initialization of this server. 
	// Hence, the cmd server must have been previously created. 
	int cmd_server_tid = WhoIs( COMMAND_SERVER_NAME );
	bwassert( cmd_server_tid >= 0, "SWITCH SERVER: This task requires the command server to be able to operate." ); 
 
	Switches_list switches_list;
	init_switches( &switches_list, cmd_server_tid, str_ptr ); 
	
	FOREVER{
		bwdebug( DBG_USR, "SWITCHES SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &cmd_request, sizeof( cmd_request ) );
		
		if ( cmd_request.type == ADD_CMD_REQUEST && cmd_request.cmd.cmd_type == SWITCH_CMD_TYPE ){
			bwdebug( DBG_USR, "SWITCHES SERVER: Received add command from [sender_tid: %d]",
					sender_tid ); 
			Reply( sender_tid, 0, 0 ); 
			
			change_switch_position( cmd_request, &switches_list, cmd_server_tid, str_ptr ); 
			
			// Update the change in the screen. 
			Putstr( COM2, str_ptr );
		}
		else{
			bwdebug( DBG_USR, "COMMAND SERVER: Invalid cmd received from [sender_tid: %d cmd: %d]",
					sender_tid, cmd_request.type );
			Reply( sender_tid, 0, 0 ); 			
		}
	}	
}

