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
	// Initialization
	bwdebug( DBG_USR, TRAIN_MGR_DEBUG_AREA, "TRAIN_MANAGER: start" );
	RegisterAs( TRAIN_MANAGER_NAME );

	// Create other necessary tasks
	int route_srv_tid = Create( ROUTE_SERVER_PRIORITY, route_server ); 
	bwdebug( DBG_USR, TRAIN_MGR_DEBUG_AREA, 
		"TRAIN_MANAGER: Created route server successfully [ route_srv_tid: %d ]", route_srv_tid );

	int track_id, num_trains, sender_tid;
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
	track_id = init_msg.track_id;
	
	if ( track_id == TRACK_ID_A ) 
		init_tracka( track ); 
	else if ( track_id == TRACK_ID_B )
		init_trackb( track );
	else
		bwassert( 0, "TRAIN_MANAGER_SERVER: The track id must be valid (A or B)" ); 

	// Initialize route server data
	init_track( track ); 

	init_reply.track = track; 
	Reply( sender_tid, ( char * ) &init_reply, sizeof( init_reply ) );

	FOREVER{
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
