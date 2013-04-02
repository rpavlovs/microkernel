#include "userspace.h"

// -------------------------------------------------------------------
// Train manager server
// -------------------------------------------------------------------
track_node *get_location_node( const char *location_name, track_node *track ){
	// Get the node by brute force
	int node_found = 0; 
	track_node *ptr = track;

	do{
		if( strcmp( location_name, ptr->name ) == 0 ){
			node_found = 1; 
			break; 
		}
	} while( ++ptr != track );

	if ( node_found )
		return ptr; 
	else
		return 0; 
}

void train_manager(){
	bwdebug( DBG_USR, TRAIN_MGR_DEBUG_AREA, "TRAIN_MANAGER: start" );
	// Initialization
	int display_tid, i;
	msg_init_track_disp init_display_msg;
	msg_display_request req_msg;
	RegisterAs( TRAIN_MANAGER_NAME );

	// Create other necessary tasks
	int route_srv_tid = Create( ROUTE_SERVER_PRIORITY, route_server ); 
	bwdebug( DBG_USR, TRAIN_MGR_DEBUG_AREA, 
		"TRAIN_MANAGER: Created route server successfully [ route_srv_tid: %d ]", route_srv_tid );

	int num_trains, sender_tid;
	num_trains = 0;							// Currently there are no trains
	int trains_tids[ NUM_TRAINS ]; 
	track_node track[ MAX_NUM_NODES_TRACK ]; 

	// Messages
	Train_manager_msg msg; 
	Train_manager_reply msg_reply; 
	Train_initialization_msg train_initialization; 

	// Initialize the track data
	// TODO: Here other tasks will access the memory of this task. If we
	// see that this is causing problems we can move this memory to 
	// the free space 0x0143BFFF
	Train_mgr_init_msg init_msg;
	Train_mgr_init_reply init_reply; 
	Receive( &sender_tid, ( char * ) &init_msg, sizeof( init_msg ) );
	
	bwassert( init_msg.track_id == TRACK_ID_A || init_msg.track_id == TRACK_ID_B,
		"train_manager: track id is unknown" );
	( init_msg.track_id == TRACK_ID_A ? init_tracka( track ) : init_trackb( track ) );

	// Initialize route server data
	init_track( track );

	init_display_msg.type = MSG_TYPE_INIT_TRACK_DISP;
	init_display_msg.track = track;
	display_tid = Create( 9, track_display );
	Send( display_tid, (char *)&init_display_msg, sizeof(init_display_msg), 0, 0 );

	req_msg.type = MSG_TYPE_DISP_SWITCH;
	req_msg.state = DIR_CURVED;
	for( i = 80; i < 123; ++i ) {
		if( i % 2 == 1 ) continue;
		req_msg.switch_id = i;		
		Send( display_tid, (char *)&req_msg, sizeof(req_msg), 0, 0 );
	}

	init_reply.track = track; 
	Reply( sender_tid, ( char * ) &init_reply, sizeof( init_reply ) );

	FOREVER {
		bwdebug( DBG_USR, TRAIN_MGR_DEBUG_AREA, "TRAIN_MANAGER_SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &msg, sizeof( msg ) );

		switch( msg.msg_type ){
			case TRAIN_MGR_ADD_TRAIN_MSG:
				// Create the train. 
				if ( num_trains < NUM_TRAINS ){
					int train_tid = Create( TRAIN_TASK_PRIORITY, train_server ); 
					trains_tids[ num_trains ] = train_tid; 
					if ( train_tid < 0 )
						bwassert( 0, "TRAIN_MANAGER_SERVER: Could not create the train server [ result: %d ]", train_tid );

					// Initialize train
					train_initialization.train_id = msg.element_id; 
					train_initialization.direction = msg.param; 
					train_initialization.track = track;
					Send( train_tid, ( char * ) &train_initialization, sizeof( train_initialization ), 0, 0  );

					msg_reply.train_tid = train_tid; 
					Reply( sender_tid, ( char * ) &msg_reply, sizeof( msg_reply ) ); 
				}
				
				break;
			// TODO: Lost message -> Stop all trains, and only move 1 train
			// TODO: Get track data
			default:
				bwassert( 0, "TRAIN_MANAGER_SERVER: Invalid request type." );
				break; 
		}
	}
}
