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
} Train_status;

typedef struct{
	Train_status trains[ TRAIN_LIST_SIZE ];
} Train_list;

void init_train_list( Train_list *list ){
	int i = 0; 
	for( i = 0; i < TRAIN_LIST_SIZE; i++ ){
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
	assert( cmd_queue->size != COMMAND_QUEUE_SIZE, "Command queue should not overflow" );
	
	cmd_queue->size++; 
	if ( ++(cmd_queue->newest) >= COMMAND_QUEUE_SIZE )
		cmd_queue->newest = 0; 
	
	// Copy the command information into the cmd in the queue. 
	Command *cmd = &(cmd_queue->cmds[ cmd_queue->newest ]); 
	cmd->cmd_type = temp_cmd.cmd_type;
	cmd->element_id = temp_cmd.element_id; 
	cmd->param = temp_cmd.param; 
}

Command dequeue_cmd( Command_queue *cmd_queue ){
	assert( cmd_queue >= 0, "Command queue should have items to dequeue." ); 
	
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
	debug( DBG_SYS, "COMMAND SERVER: enters" );
	
	// Initialization
	int cmd_server_tid; 
	Cmd_initial_msg initial_msg; 
	Cmd_request cmd_request; 
	
	Train_list train_list; 
	init_train_list( &train_list ); 
	
	debug( DBG_SYS, "COMMAND SERVER: recieving init info" );
	Receive( &cmd_server_tid, ( char * ) &initial_msg, sizeof( initial_msg ) );
	Reply( cmd_server_tid, 0, 0 ); 
	
	Command_queue *cmd_queue = initial_msg.cmd_queue; 
	FOREVER{
		if( cmd_queue->size > 0 ){
			Command command = dequeue_cmd( cmd_queue ); 
			char requested_pos; 
			
			switch( command.cmd_type ){
				case TRAIN_CMD_TYPE:		// Train
					// Keep track of the current speed. 
					train_list.trains[ command.element_id ].id = command.element_id; 
					train_list.trains[ command.element_id ].speed = command.param; 
					
					// Send commands to UART 1. 
					Putc( COM1, command.param );
					Putc( COM1, command.element_id );
					
					break; 
				case REVERSE_CMD_TYPE:		// Reverse
					bwprintf( COM2, "\033[%40;8HReversing the train/0" ); 
					/*
					// Stop train
					Putc( COM1, 0 );				// Stop the train
					Putc( COM1, command.element_id );
					Delay( INTER_CMD_DELAY ); 
					
					// Reverse train
					Putc( COM1, 15 );
					Putc( COM1, command.element_id ); 
					Delay( INTER_CMD_DELAY ); 
					
					// Restart train
					Putc( COM1, train_list.trains[ command.element_id ].speed );
					Putc( COM1, command.element_id ); 					
					*/
					break; 
				case SWITCH_CMD_TYPE:		// Switch
					requested_pos = ( char ) command.param; 
					if ( requested_pos == SWITCH_STRAIGHT_POS ){
						Putc( COM1, 33 );
					}
					else if ( requested_pos == SWITCH_CURVE_POS ){
						Putc( COM1, 34 );
					}
					else{
						assert( 0, "COMMAND SERVER: Invalid switch position. " ); 
					}
					Putc( COM1, command.element_id );
					
					// Reset the switch (so that the solenoid doesn't burn)
					Putc( COM1, 32 );
					break; 
				default: 
					debug( DBG_SYS, "COMMAND SERVER: Invalid command. Cannot execute [ Cmd: %d ]", 
							command.cmd_type ); 
					bwprintf( COM2, "Reversing the train/0" ); 
					break; 
			}
			
			//Delay( INTER_CMD_DELAY ); // This adds the delay required between commands  
		}
		else{
			// There are not commands left to send. Wait until one arrives. 
			cmd_request.type = CMD_NOTIFIER_IDLE; 
			Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0 ); 
		}
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Command Server
// -----------------------------------------------------------------------------------------------------------------------------------------------
void commandserver(){
	debug( DBG_SYS, "COMMAND SERVER: enters" );
	RegisterAs( COMMAND_SERVER_NAME );
	
	// Initialization
	int cmd_notifier_tid, sender_tid, notifier_idle = 0; 
	
	int time_server_tid = WhoIs( TIME_SERVER_NAME ); 
	assert( time_server_tid >= 0, "COMMAND SERVER: This server can't work without the time server" ); 
	
	Cmd_request cmd_request; 
	Command_queue cmd_queue; 
	init_cmd_queue( &cmd_queue ); 
	
	// Create and contact the notifier. 
	cmd_notifier_tid = Create( COMMAND_NOTIFIER_PRIORITY, command_notifier ); 
	debug( DBG_SYS, "COMMAND_SERVER: command_notifier created [tid: %d priority: %d]", 
			cmd_notifier_tid, UART_SENDER_NOTIFIER_PRIORITY );
	
	Cmd_initial_msg initial_msg; 
	initial_msg.cmd_queue = &cmd_queue; 
	Send( cmd_notifier_tid, ( char * ) &initial_msg, sizeof( initial_msg ), 0, 0  );  
	
	FOREVER{
		debug( DBG_SYS, "COMMAND SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &cmd_request, sizeof( cmd_request ) );

		switch( cmd_request.type ){
			case ADD_CMD_REQUEST:
				debug( DBG_SYS, "COMMAND SERVER: Added command from [sender_tid: %d]",
					sender_tid );
				enqueue_cmd( cmd_request.cmd, &cmd_queue ); 
				Reply( sender_tid, 0, 0 ); 
				break; 
			case CMD_NOTIFIER_IDLE:
				debug( DBG_SYS, "COMMAND SERVER: notifier is idle" );
				notifier_idle = 1; 
				break; 
			default: 
				debug( DBG_SYS, "COMMAND SERVER: Invalid cmd received from [sender_tid: %d cmd: %d]",
					sender_tid, cmd_request.type );
				Reply( sender_tid, 0, 0 ); 
				break; 
		}
		 
		if ( notifier_idle == 1 && cmd_queue.size > 0 ){
			notifier_idle = 0; 
			debug( DBG_SYS, "COMMAND SERVER: Waking up notifier" );
			Reply( cmd_notifier_tid, 0, 0 ); 
		}
	}
}

