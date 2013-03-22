#include "userspace.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Train Status
// TODO: This is temporary only. In the next assignments we need to keep track of train speed, position, etc. 
//	For now this is only needed for the reverse command. 
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define TRAIN_LIST_SIZE			100

typedef struct{
	int id; 
	int speed; 
} Train_sts;

typedef struct{
	Train_sts trains[ TRAIN_LIST_SIZE ];
} Train_list;

void init_train_list( Train_list *list ) {
	int i; 
	for( i = 0; i < TRAIN_LIST_SIZE; i++ ) {
		list->trains[ i ].id = -1; 
		list->trains[ i ].speed = 0;
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Command Queue Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
void init_cmd_queue(Command_queue *cmd_queue){
	cmd_queue->newest = -1; 
	cmd_queue->oldest = 0; 
	cmd_queue->size = 0; 
}

void enqueue_cmd( Command temp_cmd, Command_queue *cmd_queue ){
	bwassert( cmd_queue->size != CHAR_QUEUE_SIZE, "Command queue should not overflow" );
	
	cmd_queue->size++; 
	if ( ++(cmd_queue->newest) >= COMMAND_QUEUE_SIZE )
		cmd_queue->newest = 0; 
	
	// Copy the command information into the cmd in the queue. 
	Command *cmd = &(cmd_queue->cmds[ cmd_queue->newest ]); 
	cmd->cmd_type = temp_cmd.cmd_type;
	cmd->element_id = temp_cmd.element_id; 
	cmd->param = temp_cmd.param; 
	cmd->sender_tid = temp_cmd.sender_tid; 
	cmd->sensors = temp_cmd.sensors; 
}

Command dequeue_cmd( Command_queue *cmd_queue ){
	bwassert( cmd_queue->size >= 0, "dequeue_cmd: Command queue should not be empty." ); 
	
	cmd_queue->size--; 
	Command cmd = cmd_queue->cmds[ cmd_queue->oldest ]; 
	if ( ++(cmd_queue->oldest) >= COMMAND_QUEUE_SIZE )
		cmd_queue->oldest = 0; 
	
	return cmd; 
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Command Notifier
// -- NOTE: The notifier requires time server to work properly. 
// -----------------------------------------------------------------------------------------------------------------------------------------------
void command_notifier(){
	bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: enters" );
	
	// Initialization
	int cmd_server_tid; 
	Cmd_initial_msg initial_msg; 
	Cmd_request cmd_request; 
	
	Train_list train_list; 
	init_train_list( &train_list ); 
	
	bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: recieving init info" );
	Receive( &cmd_server_tid, ( char * ) &initial_msg, sizeof( initial_msg ) );
	Reply( cmd_server_tid, 0, 0 ); 
	
	Command_queue *cmd_queue = initial_msg.cmd_queue; 
	FOREVER{
		if( cmd_queue->size > 0 ){
			Command command = dequeue_cmd( cmd_queue ); 
			char requested_pos; 
			
			switch( command.cmd_type ){
				case TRAIN_CMD_TYPE:		// Train
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: send tr" );
					
					// Keep track of the current speed. 
					train_list.trains[ command.element_id ].id = command.element_id; 
					train_list.trains[ command.element_id ].speed = command.param; 
					
					// Send commands to UART 1. 
					//Putc( COM1, command.param );
					//Putc( COM1, command.element_id );
					
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: finished tr command" );
					break; 
				case REVERSE_CMD_TYPE:		// Reverse
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: send rv" );
					
					// Stop train
					//Putc( COM1, 0 );				// Stop the train
					//Putc( COM1, command.element_id );
					Delay( REVERSE_CMD_DELAY );		// Wait some time to avoid damaging the trains. 
					
					// Reverse train
					//Putc( COM1, 15 );
					//Putc( COM1, command.element_id ); 
					
					// Restart train
					//Putc( COM1, train_list.trains[ command.element_id ].speed );
					//Putc( COM1, command.element_id ); 
					
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: finished rv command" );
					break; 
				case SWITCH_CMD_TYPE:		// Switch
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: send sw" );

					requested_pos = ( char ) command.param; 
					if ( requested_pos == SWITCH_STRAIGHT_POS ){
						//Putc( COM1, 33 );
					}
					else if ( requested_pos == SWITCH_CURVE_POS ){
						//Putc( COM1, 34 );
					}
					else{
						bwassert( 0, "COMMAND SERVER: Invalid switch position. " ); 
					}
					//Putc( COM1, command.element_id );
					
					// Reset the switch (so that the solenoid doesn't burn)
					//Putc( COM1, 32 );

					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: finished sw command" );
					break; 
				case RESET_SENSORS_CMD_TYPE:
					// Simply execute the reset code.
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: reset sensors" );

					//Putc( COM1, RESET_CODE );
					Reply( command.sender_tid, 0, 0 ); 
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: finished resettting sensors command" );
					break; 
				case QUERY_SENSORS_CMD_TYPE:
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: query sensors" );
					
					// Init
					char *sensors = command.sensors; 
					
					// First request the data
					Delay( 1 ); 
					//Putc( COM1, REQUEST_DATA_CODE );

					// Retrieve and return the sensors data. 
					int i;
					for( i = 0; i < 10; ++i ) {
						//sensors[i] = Getc( COM1 );
					}
					Reply( command.sender_tid, 0, 0 ); 
					
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: finished query sensors command" );
					break; 
				default: 
					bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: Invalid command. Cannot execute [ Cmd: %d ]", 
							command.cmd_type ); 
					break; 
			}
		}
		else{
			// There are not commands left to send. Wait until one arrives. 
			cmd_request.type = CMD_NOTIFIER_IDLE;
			bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_NOTIFIER: report idle" );
			Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0 ); 
		}
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Command Server
// -----------------------------------------------------------------------------------------------------------------------------------------------
void commandserver(){
	bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: enters" );
	RegisterAs( COMMAND_SERVER_NAME );
	
	// Initialization
	int cmd_notifier_tid, sender_tid, notifier_idle = 0; 
	
	int time_server_tid = WhoIs( TIME_SERVER_NAME ); 
	bwassert( time_server_tid >= 0, "COMMAND SERVER: This server can't work without the time server" ); 
	
	Cmd_request cmd_request; 
	Command_queue cmd_queue; 
	init_cmd_queue( &cmd_queue ); 
	
	// Create and contact the notifier. 
	cmd_notifier_tid = Create( COMMAND_NOTIFIER_PRIORITY, command_notifier ); 
	bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND_SERVER: command_notifier created [tid: %d priority: %d]", 
			cmd_notifier_tid, UART_SENDER_NOTIFIER_PRIORITY );
	
	Cmd_initial_msg initial_msg; 
	initial_msg.cmd_queue = &cmd_queue; 
	Send( cmd_notifier_tid, ( char * ) &initial_msg, sizeof( initial_msg ), 0, 0  );  
	
	FOREVER{
		bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &cmd_request, sizeof( cmd_request ) );

		switch( cmd_request.type ){
			case ADD_CMD_REQUEST:
				bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: Added command from [cmd: %d sender_tid: %d]",
					cmd_request.cmd, sender_tid );
				enqueue_cmd( cmd_request.cmd, &cmd_queue ); 
				Reply( sender_tid, 0, 0 ); 
				break; 
			case QUERY_CMD_REQUEST:
				bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: Added query command from [sender_tid: %d]",
					sender_tid );
				enqueue_cmd( cmd_request.cmd, &cmd_queue ); 
				// NOTE: This command requires returning data. Hence, there's no reply here.
				break; 
			case CMD_NOTIFIER_IDLE:
				bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: notifier is idle" );
				notifier_idle = 1; 
				break; 
			default: 
				bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: Invalid cmd received from [sender_tid: %d cmd: %d]",
					sender_tid, cmd_request.type );
				Reply( sender_tid, 0, 0 ); 
				break; 
		}
		 
		if ( notifier_idle == 1 && cmd_queue.size > 0 ){
			notifier_idle = 0; 
			bwdebug( DBG_USR, COMMAND_SERVER_DEBUG_AREA, "COMMAND SERVER: Waking up notifier" );
			Reply( cmd_notifier_tid, 0, 0 ); 
		}
	}
}

