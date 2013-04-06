#include <userspace.h>

// -------------------------------------------------------------------
// Queue methods
// -------------------------------------------------------------------
void init_train_cmd_queue( Train_cmd_queue *q ){
	q->newest = -1;
	q->oldest = 0;
	q->size = 0;
}

void train_cmd_queue_push( Train_cmd cmd, Train_cmd_queue *q ){
	bwassert(q->size != TRAIN_COMMAND_QUEUE_SIZE, "train_cmd_queue_push: Train command queue should not overflow");

	// Enqueue the command
	if( ++( q->newest ) == TRAIN_COMMAND_QUEUE_SIZE ) q->newest = 0;
	q->commands[q->newest] = cmd;	// TODO: Do we need to copy "manually" these values? 
	q->size++;
}

Train_cmd train_cmd_queue_pop( Train_cmd_queue *q ){
	bwassert( q->size > 0, "train_cmd_queue_pop: Train command queue should not be empty." ); 

	// Dequeue the command
	Train_cmd cmd = q->commands[q->oldest];
	if( ++( q->oldest ) == TRAIN_COMMAND_QUEUE_SIZE ) q->oldest = 0;
	q->size--;
	return cmd;
}

// -------------------------------------------------------------------
// Train command notifier
// This notifier is used to send all the train commands that need to 
// be delayed. The other commands will be sent directly to the right
// servers. 
// -------------------------------------------------------------------
void train_cmd_notifier(){
	// Initialization
	bwdebug( DBG_USR, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_CMD_NOTIFIER: Enters" );

	// Variables
	int current_time, time_to_send_cmd, train_server_tid, cmd_server_tid; 

	// Getting command server information
	cmd_server_tid = WhoIs( COMMAND_SERVER_NAME ); 
	bwassert( cmd_server_tid, "TRAIN_CMD_NOTIFIER: This notifier needs the command server to work properly" ); 

	// Init message
	Train_cmd_notifier_msg msg;
	msg.type = TRAIN_CMD_INIT_MSG_TYPE;
	bwdebug( DBG_SYS, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_CMD_NOTIFIER: recieving init info" );
	Receive( &train_server_tid, ( char * ) &msg, sizeof( msg ) );
	Reply( train_server_tid, 0, 0 );

	Train_cmd_queue *queue = msg.queue;	

	FOREVER{
		// If nothing to send notify server and block 
		if( queue->size > 0 ) {
			Train_cmd cmd = train_cmd_queue_pop( queue );

			if ( cmd.delay_until > 0 ){
				// NOTE: Remember that the time of delay until is in MS and this is in ticks. 
				current_time = Time(); 
				time_to_send_cmd = cmd.delay_until / 10; 
				if ( time_to_send_cmd > current_time ){
					
					//bwprintf( COM2, "TRAIN_CMD_NOTIFIER: Delaying until: %d Cmd Until: %d\n", time_to_send_cmd, cmd.delay_until ); 
					bwdebug( DBG_SYS, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_CMD_NOTIFIER: Delaying [ Delay Until:  %d ]", time_to_send_cmd );
					DelayUntil( time_to_send_cmd );
				}
			}
			//bwprintf( COM2, "TRAIN_CMD_NOTIFIER: Delaying until: %d Cmd Until:%d\n", time_to_send_cmd, cmd.delay_until ); 
			bwprintf( COM2, "\nTRAIN_CMD_NOTIFIER: Sending cmd\n - [ Ticks: %d Ms: %d Train: %d Speed: %d ]\n", 
				cmd.delay_until / 10, cmd.delay_until, cmd.request.cmd.element_id, cmd.request.cmd.param ); 
			Send( cmd_server_tid, ( char * ) &cmd.request, sizeof( cmd.request ), 0, 0 ); 
		} 
		else {
			bwdebug( DBG_SYS, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_CMD_NOTIFIER: blocking until buffer refilled" );
			msg.type = TRAIN_CMD_IDLE_MSG_TYPE;
			Send( train_server_tid, ( char * ) &msg, sizeof( msg ), 0, 0 );
			bwdebug( DBG_SYS, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_CMD_NOTIFIER: waking up" );
		}
	}
}
