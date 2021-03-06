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

int reserve_distance( int distance_to_reserve, Train_status *train_status, Train_server_data *server_data ){
	// TODO: Add the reservation code here. 
	return 1; 
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
	temp_buffer += cursorPositioning( temp_buffer, TRAIN_STATUS_ROW_POS + train_status->train_num, TRAIN_STATUS_COL_POS );
	temp_buffer += sprintf( temp_buffer, "Id -> %d   Direction -> %s   Landmark -> %s", 
		train_status->train_id, 
		( train_status->train_direction == TRAIN_DIRECTION_FORWARD ) ? "FOR" : "REV", train_status->current_position.landmark->name ); 
	temp_buffer += append_char( temp_buffer, ' ', 5 - strlen( train_status->current_position.landmark->name ) );
	temp_buffer += sprintf( temp_buffer, 
		" Offset -> %d ", train_status->current_position.offset ); 
	temp_buffer += sprintf( temp_buffer, "   Speed -> %d   Error -> %d", train_status->motion_data.train_speed, train_status->motion_data.current_error ); 
	temp_buffer += restoreCursor( temp_buffer );

	// Send the string to UART 2. 
	//Putstr( COM2, buff );
}

/*
  This method is used to update the reference landmark when a reverse command is issued. 
*/
int reverse_current_position( Train_status *train_status ){

	// 1. Change the landmark from the source to the destination. 
	train_status->current_position.landmark = train_status->current_position.edge->dest->reverse; 
        
	// 2. Update the offset. 
	int offset = train_status->current_position.offset; 
	offset = train_status->current_position.edge->dist - offset; 
	train_status->current_position.offset = offset; 

	// 3. Change the edge. 
	// TODO: Is this the right way to do this? 
	train_status->current_position.edge = train_status->current_position.edge->reverse; 

	return 0; 
}

void clear_train_motion_data( Train_status *train_status ){
	// TODO: Check if this is correct. 
	train_status->is_reversing = 0; 
	train_status->distance_since_speed_change = 0; 
	train_status->distance_before_deacceleration = 0; 

	train_status->motion_data.train_speed = 0; 
	train_status->motion_data.current_error = 0; 
	train_status->motion_data.distance_to_travel = 0; 
	train_status->motion_data.original_train_speed = 0; 
	train_status->motion_data.calibrated_dist_index = 0; 
	train_status->motion_data.distance_type = LONG_DISTANCE; 
	train_status->motion_data.time_since_deacceleration = 0; 
}

void initialize_goal( Train_status *train_status ){
	// If there was a goal, erase it. 
	train_status->train_state = TRAIN_STATE_MOVE_FREE; 

	// Make sure that the current "goal" is empty
	train_status->current_goal.edge = 0; 
    train_status->current_goal.landmark = 0; 
    train_status->current_goal.offset = 0;

	clear_train_motion_data( train_status ); 
}

int requires_reversing( Train_status *train_status ){
	track_node *current_node, *next_node;

	int num_landmarks = train_status->route_data.num_landmarks; 
	int landmarks_index = train_status->route_data.landmark_index; 

	if ( landmarks_index + 1 < num_landmarks ){
		// Two nodes are required to mark reverse. 
		current_node = train_status->route_data.landmarks[ landmarks_index ];
		next_node = train_status->route_data.landmarks[ landmarks_index + 1 ];

		if ( current_node->reverse == next_node )
			return 1; 
	}

	return 0; 
}

int check_next_sw_pos( int distance_to_check, Train_status *train_status, Train_server_data *server_data ){
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_HELPERS: check_next_sw_pos Checking if a sw needs to be switched" );
	
	// Initialization
	char temp_str[10];
	track_node *node; 
	track_edge *edge; 
	char expected_sw_pos, real_sw_pos; 
	int num_sw_changed;
	int landmark_index, curr_sw_index; 
	
	// Messages
	Cmd_request cmd_request;
    cmd_request.type = QUERY_CMD_REQUEST; 
    cmd_request.cmd.cmd_type = SWITCH_STATE_CMD_TYPE; 

	Cmd_request change_request; 
	change_request.type = ADD_CMD_REQUEST; 
	change_request.cmd.cmd_type = SWITCH_CMD_TYPE; 

    Switch_query_reply query_msg_reply; 

	// Servers
	int sw_server_tid = server_data->tasks_tids[TR_SWITCH_SERVER_TID_INDEX]; 
	
	// Remove the distance that we have already traveled. 
	landmark_index = train_status->route_data.landmark_index;
	distance_to_check -= ( train_status->route_data.edges[ landmark_index ]->dist - train_status->current_position.offset );

	// Is there a switch in our route within the reserved distance?
	landmark_index++; 
	//bwprintf( COM2, "--------------------------------------------------------------------\n" ); 
	//bwprintf( COM2, "CHECK_NEXT_SW_POS: distance_to_check: %d current_landmark: %s pos: %d -\n", 
	//	distance_to_check, train_status->current_position.landmark->name, train_status->current_position.offset ); 
	while( distance_to_check > 0 && landmark_index < train_status->route_data.num_landmarks ){
		//bwdebug( DBG_USR, TEMP_DEBUG_AREA, "SW num_landmarks: %d landmark_index: %d distance_to_check: %d", 
		//	train_status->route_data.num_landmarks, landmark_index, distance_to_check ); 
		// Get the next node and edge
		node = train_status->route_data.landmarks[landmark_index]; 
		edge = train_status->route_data.edges[ landmark_index - 1 ];

		// A switch can be represented by two types of nodes: BRANCH and MERGE.
		// Both need to be considered
		if ( node->type == NODE_BRANCH || node->type == NODE_MERGE ){
			// Query the current state of the switch
			int switch_id = atoi( substr( temp_str, node->name, 2, 6 ) );
			cmd_request.cmd.element_id = switch_id; 

			Send( sw_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 
				( char * ) &query_msg_reply, sizeof( query_msg_reply ) );

			if ( node->type == NODE_BRANCH ){
				// Is the position of the switch like the one specified in the route? 
				curr_sw_index = get_switch_index( switch_id ); 
				expected_sw_pos = ( char ) train_status->route_data.switches_state[ curr_sw_index ];
				real_sw_pos = query_msg_reply.switch_position;
			}
			else{
				// Is the position of the switch in the direction the train is coming from?
				expected_sw_pos = ( node->reverse->edge[DIR_STRAIGHT].reverse == edge ) ? SWITCH_STRAIGHT_POS : SWITCH_CURVE_POS; 
				real_sw_pos = query_msg_reply.switch_position; 
			}

			// Make the switch change
			if ( expected_sw_pos != real_sw_pos ){
				bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
					"TRAIN_HELPERS: check_next_sw_pos -> SW needs to be changed [ sw_id: %d current_pos: %c pos: %c ]", 
					switch_id, real_sw_pos, expected_sw_pos );

				//bwprintf( COM2, "SW_POS: SW needs change [ sw_id: %d current_pos: %c pos: %c NODE: %d Index: %d ]\n", 
				//	switch_id, real_sw_pos, expected_sw_pos, node->type, landmark_index ); 

				change_request.cmd.element_id = switch_id;
				change_request.cmd.param = expected_sw_pos; 
				Send( sw_server_tid, ( char * ) &change_request, sizeof( change_request ), 0, 0 ); 

				num_sw_changed++; 
			}
		}

		edge = train_status->route_data.edges[ landmark_index ];
		if ( landmark_index + 1 < train_status->route_data.num_landmarks && edge )
			distance_to_check -= edge->dist;
		landmark_index++; 
	}
	
	return num_sw_changed; 
}

int get_short_distance( int distance_index, Speed_calibration_data *speed_calibration ){
	const int **calibrated_distances = speed_calibration->calibrated_distances;
	int offset = ( distance_index * 3 ) + CALIBRATED_DISTANCE_INDEX;

	return ( int ) *( calibrated_distances + offset );

	//calibrated_distances + ( distance_index * 3 ) +

	//return ( int ) ( &calibrated_distances[distance_index] )[ CALIBRATED_DISTANCE_INDEX ];
	//return *( calibrated_distances + offset );
}

int get_short_distance_stopping_time( int distance_index, Speed_calibration_data *speed_calibration ){
	const int **calibrated_distances = speed_calibration->calibrated_distances;
	int offset = ( distance_index * 3 ) + CALIBRATED_STOP_TIME_INDEX;

	return ( int ) *( calibrated_distances + offset );
}

int get_short_distance_total_time( int distance_index, Speed_calibration_data *speed_calibration ){
	const int **calibrated_distances = speed_calibration->calibrated_distances;
	int offset = ( distance_index * 3 ) + CALIBRATED_TOTAL_TIME_INDEX;

	return ( int ) *( calibrated_distances + offset );
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

/*
	This method returns the moment in time when a train will reach constant speed. 
*/
int get_time_to_constant_speed( Train_status *train_status, Train_server_data *server_data ){
	int time_to_speed, train_speed; 
	
	time_to_speed = 0; 
	train_speed = train_status->motion_data.train_speed;
	if ( train_speed > 0 ){
		time_to_speed = 
			server_data->calibration_data.speed_data[ train_speed - 1 ].time_to_constant_speed; //+ train_status->time_speed_change;

		bwassert( time_to_speed > 0, 
			"TRAIN_HELPERS: time_to_reach_constant_speed -> The time to get a constant speed must always be more than 0" ); 
	}

	return time_to_speed; 
}

int get_time_to_start_deaccelerating( Train_status *train_status, Train_server_data *server_data ){
	// Variables declaration
	int result; 
	int train_speed, distance_to_travel, stopping_distance, acceleration_distance;

	float distance_in_constant_speed, time_in_constant_speed; 

	// When is the train supposed to start de-accelerating? 
	train_speed = train_status->motion_data.train_speed;
	distance_to_travel = train_status->motion_data.distance_to_travel; 
	stopping_distance = get_train_stopping_distance( train_speed, server_data ); 
	acceleration_distance = get_train_acceleration_distance( train_speed, server_data );
	distance_in_constant_speed = distance_to_travel - stopping_distance - acceleration_distance;
	time_in_constant_speed = get_time_for_distance( distance_in_constant_speed, train_speed, server_data );
	result = round_decimal_up( get_time_to_constant_speed( train_status, server_data ) + time_in_constant_speed );

	/*
	bwprintf( COM2, "TIME_TO_DEACCELERATE ->\n" ); 
	bwprintf( COM2, "   -dist_travel: %d speed_ch: %d \n   -stop_dist: %d dist_csp: %d \n", 
		distance_to_travel, train_status->time_speed_change, stopping_distance, ( int ) distance_in_constant_speed ); 
	bwprintf( COM2, "   -time_csp: %d time_to_csp: %d \n   -result: %d dist_so_far: %d\n", 
		( int ) time_in_constant_speed, 
		get_time_to_constant_speed( train_status, server_data ),
		result, train_status->distance_since_speed_change ); 
	*/
	/*
	bwdebug( DBG_USR, TEMP_DEBUG_AREA, 
		"TIME_TO_DEACCELERATE [ distance_travel: %d stopping_distance: %d distance_csp: %d time_in_csp: %d time_to_csp: %d result: %d ]", 
		distance_to_travel, stopping_distance, ( int ) distance_in_constant_speed, ( int ) time_in_constant_speed, 
		get_time_to_constant_speed( train_status, server_data ),
		result );
		*/

	return result; 
}

inline int get_next_update_time( int current_time ){
	return current_time + ( WAIT_NOTIFIER_DELAY * 10 );
}

inline int get_train_stopping_distance( int train_speed, Train_server_data *server_data ){
	return server_data->calibration_data.speed_data[ train_speed - 1 ].stopping_distance; 
}

inline int get_train_acceleration_distance( int train_speed, Train_server_data *server_data ){
	return server_data->calibration_data.speed_data[ train_speed - 1 ].distance_during_acceleration; 
}

inline int get_reservation_distance_buffer( Train_server_data *server_data ){
	// TODO: It might be a good idea to make this flag configurable from the CLI.
	return RESERVATION_DISTANCE_BUFFER; 
}

inline int get_sw_delay_time( Train_server_data *server_data ){
	// TODO: It might be a good idea to make this flag configurable from the CLI.
	return SW_CHANGE_DELAY;
}

inline int get_reverse_delay_time( Train_server_data *server_data ){
	// TODO: It might be a good idea to make this flag configurable from the CLI.
	return REVERSE_CMD_DELAY;
}

inline int round_decimal_up( float value ){
	value += .9999; // It works more like a roof than a rounding function. 
	return ( int ) value; 
}
