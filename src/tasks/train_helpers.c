#include <userspace.h>

// ----------------------------------------------------------------------------------------------
// Train Helpers
// ----------------------------------------------------------------------------------------------
int request_new_path( Train_status *train_status, Train_server_data *server_data ){
    // 1. Get the current state of all sensors
	int switch_server_tid = server_data->tasks_tids[ TR_SWITCH_SERVER_TID_INDEX ];
    Cmd_request cmd_request;
    cmd_request.type = QUERY_CMD_REQUEST; 
    cmd_request.cmd.cmd_type = ALL_SWITCHES_STATE_CMD_TYPE;
    cmd_request.cmd.element_id = 0;
    cmd_request.cmd.param = ( int ) &( train_status->route_data.switches_state ); // CAREFUL!!!
    Send( switch_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  );

	// 2. Send a request to the route server. 
	Route_msg route_msg;
	int route_found; 
	
	// Basic data
	route_msg.type = GET_SHORTEST_ROUTE_MSG;
	route_msg.track = server_data->track;
	route_msg.train_direction = &train_status->train_direction; 
	
	// Current Position
	route_msg.current_landmark = train_status->current_position.landmark; 
	route_msg.train_shift = train_status->current_position.offset; 
	
	// Goal
	route_msg.target_node = train_status->current_goal.landmark; 
	route_msg.target_shift = train_status->current_goal.offset; 

	// Returned data	
	route_msg.route_found = &route_found;
	route_msg.switches = train_status->route_data.switches_state;
	route_msg.num_landmarks = &train_status->route_data.num_landmarks;
	route_msg.edges = ( track_edge ** ) train_status->route_data.edges; 
	route_msg.landmarks = ( track_node ** ) train_status->route_data.landmarks;

	Send( server_data->tasks_tids[ TR_ROUTE_SERVER_TID_INDEX ], 
		( char * ) &route_msg, sizeof( route_msg ), 0, 0 ); 

	// Temp
	/*
	int j; 
	for ( j = 0; j < *route_msg.num_landmarks; j++ ){
		//bwprintf( COM2, "%s:%d ", route_msg.landmarks[j]->name, route_msg.edges[j]->dist );
		bwprintf( COM2, "%s:%d ", train_status->route_data.landmarks[j]->name, train_status->route_data.edges[j]->dist );
	}

	while( 1 )
		;
		*/

	// 3. Make sure the route data state is correct
	train_status->route_data.landmark_index = 0; 

	return route_found; 
}

void print_train_status( Train_status *train_status ){
	// Initialization
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_SERVER: Printing status[ train_id: %d ]", train_status->train_id );

	char buff[500];
	char *temp_buffer = buff; 

	// Prepare the information to print
	temp_buffer += saveCursor( temp_buffer ); 
	temp_buffer += hideCursor( temp_buffer );
	temp_buffer += cursorPositioning( temp_buffer, TRAIN_STATUS_ROW_POS, TRAIN_STATUS_COL_POS );
	temp_buffer += sprintf( temp_buffer, "Id -> %d   Direction -> %s   Landmark -> %s", 
		train_status->train_id, 
		( train_status->train_direction == TRAIN_DIRECTION_FORWARD ) ? "FOR" : "REV", train_status->current_position.landmark->name ); 
	temp_buffer += append_char( temp_buffer, ' ', 5 - strlen( train_status->current_position.landmark->name ) );
	temp_buffer += sprintf( temp_buffer, 
		" Offset -> %d ", train_status->current_position.offset ); 
	temp_buffer += restoreCursor( temp_buffer );

	// Send the string to UART 2. 
	Putstr( COM2, buff );
}

/*
  This method is used to update the reference landmark when a reverse command is issued. 
*/
int reverse_current_position( Train_status *train_status ){

	// 1. Change the landmark from the source to the destination. 
	train_status->current_position.landmark = train_status->current_position.edge->dest; 
        
	// 2. Update the offset. 
	int offset = train_status->current_position.offset; 
	offset = train_status->current_position.edge->dist - offset; 
	train_status->current_position.offset = offset; 

	// 3. Change the edge. 
	// TODO: Is this the right way to do this? 
	train_status->current_position.edge = train_status->current_position.edge->reverse; 

	return 0; 
}

void initialize_goal( Train_status *train_status ){
	// If there was a goal, erase it. 
	train_status->train_state = TRAIN_STATE_MOVE_FREE; 

	// Make sure that the current "goal" is empty
	train_status->current_goal.edge = 0; 
    train_status->current_goal.landmark = 0; 
    train_status->current_goal.offset = 0; 
}

int get_short_distance( int train_speed, Speed_calibration_data *speed_calibration ){
	return speed_calibration->calibrated_distances[ train_speed ][ CALIBRATED_DISTANCE_INDEX ]; 
}

int get_short_distance_stopping_time( int train_speed, Speed_calibration_data *speed_calibration ){
	return speed_calibration->calibrated_distances[ train_speed ][ CALIBRATED_STOP_TIME_INDEX ];
}

int get_short_distance_total_time( int train_speed, Speed_calibration_data *speed_calibration ){
	return speed_calibration->calibrated_distances[ train_speed ][ CALIBRATED_TOTAL_TIME_INDEX ];
}

track_node *get_prev_node( Train_position *train_pos ){
	int landmark_type = train_pos->landmark->type;
	track_node* landmark = train_pos->landmark;
	if ( landmark_type == NODE_SENSOR || landmark_type == NODE_EXIT || landmark_type == NODE_BRANCH ){
		return landmark->reverse->edge[DIR_AHEAD].dest;
	}
	
	// If the track node is an enter there's no previous node.
	// If the track node is a merge, there's no way of knowing which is the right path. 
	return 0; 
}
