#include <userspace.h>

// -------------------------------------------------------------------
// Couriers
// -------------------------------------------------------------------
/*
  This method creates a courier. It runs on the caller's 
  Returns:
  - The tid of the newly created courier. 
*/
int create_courier( int client_id, int notifier_id, int message_type_client, int message_type_notifier ){
	// Initialization
	// NOTE: We are not adding information to be shared between client and notifier here. If
	// there's data that needs to be sent, data structures can be sent during initialization. 
	int courier_tid; 
	Courier_init_msg courier_init_msg; 
	courier_init_msg.client_tid = client_id; 
	courier_init_msg.notifier_tid = notifier_id; 
	courier_init_msg.message_type_client = ( message_type_client ) ? message_type_client : COURIER_MSG_REPLY; 
	courier_init_msg.message_type_notifier = ( message_type_notifier ) ? message_type_notifier : COURIER_MSG_SEND; 

	// Create the courier
	courier_tid = Create( COURIER_TASK_PRIORITY, courier_exec ); 
	bwdebug( DBG_USR, COURIER_DEBUG_AREA, "Courier created [ tid: %d client_tid: %d notifier_tid: %d ]", 
		courier_tid, client_id, notifier_id ); 

	// Provide the courier with initialization info.
	Send( courier_tid, ( char * ) &courier_init_msg, sizeof( courier_init_msg ), 0, 0 ); 

	return courier_tid; 
}

void courier_exec(){
	// Initialization
	int client_tid, notifier_tid, tid, reply_length; 
	char courier_buffer[ COURIER_BUFFER_SIZE ]; 
	Courier_msg msg; 
	Courier_init_msg init_msg; 

	Receive( &tid, ( char * ) &init_msg, sizeof( init_msg ) );
	Reply( tid, 0, 0 ); 
	client_tid = init_msg.client_tid; 
	notifier_tid = init_msg.notifier_tid; 

	FOREVER{
		// Will always fetch something from the notifier, and send it to the client. 
		reply_length = Send( notifier_tid, ( char * ) &msg, sizeof( msg ), courier_buffer, COURIER_BUFFER_SIZE );
		Send( client_tid, courier_buffer, reply_length, 0, 0 );
	}
}
