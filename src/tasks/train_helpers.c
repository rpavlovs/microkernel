#include <userspace.h>

// -------------------------------------------------------------------
// Movement Methods
// -------------------------------------------------------------------

/*
  This method returns the distance traveled since the last refresh. 
  NOTES: 
  - The difference with the short distance traveled is that this method
    doesn't take into account the distance required to de-accelerate. 
  - This method must be used for long distances (at least twice the distance
    required to de-accelerate at this speed)
*/
int get_long_distance_traveled( int time_since_change, 
						  Train_status *train_status, Calibration_data *calibration_data ){
	// NOTE: 
	// - Do we need to give some sort of delay between when the command was issued and when it was executed?

	int distance_traveled = 0; 
	Train_speed current_speed = train_status->current_speed; 
	Speed_calibration_data *speed_calibration = &(calibration_data->speed_data[current_speed.train_speed]); 

	if ( train_status->motion_state == TRAIN_ACCELERATING || train_status->motion_state == TRAIN_CONSTANT_SPEED ){
		if ( time_since_change <= speed_calibration->time_to_constant_speed ){
			// The train is accelerating, but the acceleration is not constant. However, since the 
			// distance is long we don't care about such small changes. We only care about the distance 
			// and time it will take to completely accelerate.
			distance_traveled = 
				( speed_calibration->distance_during_acceleration * time_since_change ) / ( speed_calibration->time_to_constant_speed ); 

			// Has the train reached constant speed? 
			if ( time_since_change == speed_calibration->time_to_constant_speed ){
				train_status->motion_state = TRAIN_CONSTANT_SPEED; 
			}
		}
		else {
			// The train has already reached constant speed. Therefore we need to sum the distance 
			// traveled during acceleration and the distance traveled on constant speed. 
			int time_constant_speed = time_since_change - speed_calibration->time_to_constant_speed;
			distance_traveled = speed_calibration->distance_during_acceleration; 
			distance_traveled += time_constant_speed * speed_calibration->velocity; 
		}
	}
	else if ( train_status->motion_state == TRAIN_DEACCELERATING ){
		// Since we don't know the (de) acceleration rate, but only the stopping distance and stopping time.
		// Even though it's not completely accurate, we just make a linear interpolation. 
		
		// Previous speed
		distance_traveled = train_status->distance_before_deacceleration;

		// Speed during de-acceleration. 
		int time = speed_calibration->stopping_time -  time_since_change; 
		if ( time > 0 ){
			// The train is still de-accelerating. 
			distance_traveled += 
				( speed_calibration->stopping_distance * time_since_change  ) / ( speed_calibration->stopping_time );
		}
		else{
			// The train has already stopped. 
			distance_traveled += speed_calibration->stopping_distance; 
			train_status->motion_state = TRAIN_STILL; 
		}
	}
	else {
		// Wrong state. 
		bwassert( 0, "TRAIN_HELPERS: get_long_distance_traveled -> Wrong train state." );
	}

	// distance_traveled contains the distance since the speed change. Now this distance is compared to the 
	// one calculated during the previous refresh. 
	// TODO: After checking the sensors remember to correctly update this parameters to avoid having negative values. 
	int distance_since_change = distance_traveled - train_status->distance_since_speed_change;
	bwassert( distance_since_change >= 0, "TRAIN_HELPERS: get_long_distance_traveled -> The distance cannot be negative." ); 

	// Update state
	train_status->distance_since_speed_change = distance_traveled;

	return distance_since_change; 
}

/*
  This method returns the distance traveled since the last refresh. 
  NOTES:
  - This method takes into account the distance required to de-accelerate. Hence, 
    it should only be used for small distances where we don't know how long it
	takes the system to accelerate, and how long it takes to deaccelerate, but we
	have an idea of how long the whole movement takes. 
*/
int get_short_distance_traveled( int time_since_change, 
						  Train_status *train_status, Calibration_data *calibration_data ){

	int distance_traveled = 0; 
	if ( train_status->motion_state == TRAIN_ACCELERATING || 
		train_status->motion_state == TRAIN_CONSTANT_SPEED || 
		train_status->motion_state == TRAIN_DEACCELERATING ){
			// The train is moving. We don't care about how long it takes to get to constant
			// speed if ever, or how long it takes to deaccelerate. The distance is very short
			// so we only work in terms of total distance and total speed. 
			int distance_traveled, time, total_distance_in_speed; 
			Train_speed current_speed = train_status->current_speed; 
			Speed_calibration_data *speed_calibration = &(calibration_data->speed_data[current_speed.train_speed]); 

			time = current_speed.total_time_in_speed - time_since_change; 
			total_distance_in_speed = speed_calibration->calibrated_distances[ current_speed.train_speed ]; 

			if ( time > 0 ){
				// Train is still moving. 
				distance_traveled = 
					( total_distance_in_speed * time_since_change ) / ( current_speed.total_time_in_speed );
			}
			else{
				// The train has already stopped
				distance_traveled = total_distance_in_speed; 
				train_status->motion_state = TRAIN_STILL; 
			}
	}
	else {
		// Wrong state. 
		bwassert( 0, "TRAIN_HELPERS: get_short_distance_traveled -> Wrong train state." );
	}

	// Get the distance traveled since last refresh
	int distance_since_change = distance_traveled - train_status->distance_since_speed_change;
	bwassert( distance_since_change >= 0, 
		"TRAIN_HELPERS: get_short_distance_traveled -> The distance cannot be negative." ); 

	// Update state
	train_status->distance_since_speed_change = distance_traveled;

	return distance_traveled; 
}

/*
  This method returns the distance traveled since the last refresh. 
*/
int get_distance_traveled( int current_time, 
						  Train_status *train_status, Calibration_data *calibration_data ){

	// NOTE:
    // Because of the nature of our project we don't change between speeds -> The only 
	// change from speed 0 to another speed, and then, at some point, we go back to 0.
	int time_since_change = current_time - train_status->time_speed_change;
	if ( time_since_change <= 0 ){
		return 0; 
	}

	Train_speed current_speed = train_status->current_speed; 

	if ( current_speed.speed_type == SHORT_DISTANCE_SPEED )
		return get_short_distance_traveled( time_since_change, train_status, calibration_data ); 
	else
		return get_long_distance_traveled( time_since_change, train_status, calibration_data ); 
}

/*
  This method returns the distance that a particular speed requires to stop completely. 
*/
int get_stopping_distance( int speed ){
	return 0; 
}

/*
  This method calculates the total straight distance that the train must travel while following it's
  route. A straight distance is defined as the distance that the train must travel without stopping. 
  The train might stop because of two reasons: it has reached its destination or it must reverse. 
  Return: 
  - The total straight distance. 
*/
int get_straight_distance( Train_status *status, int *requires_reverse ){
	// Initialization
	int is_reverse = 0; 
	int total_straight_distance = 0; 
	int landmark_index = status->route_data.landmark_index; 
	int num_landmarks = status->route_data.num_landmarks; 

	bwassert( landmark_index < num_landmarks, 
		"TRAIN_HELPERS: get_straight_distance -> The landmark index must be less than the num of landmarks." ); 

	track_edge *edge = ( track_edge * ) &( status->route_data.edges[ landmark_index ] ); 
	track_node *landmark = ( track_node * ) &( status->route_data.landmarks[ landmark_index ] ); 

	// The first edge distance must be added only if it has more than one landmark left in the route. 
	if ( landmark_index + 1 < num_landmarks )
		total_straight_distance = status->route_data.edges[ landmark_index ].dist; 

	// Remove the distance that the train has already moved from the initial landmark. 
	total_straight_distance -= status->current_position.offset; 

	// Get the distance of the next landmarks without reverse (reverse would require the train to stop).
	// NOTE: Here we only count FULL edges; the final offset will be considered later. That's the reason
	// why we are comparing the num_landmarks to landmark_index + 2. 
	while( landmark_index + 2 < num_landmarks ){
		landmark = ( track_node * ) &( status->route_data.landmarks[ landmark_index + 1 ] ); 
		edge = ( track_edge * ) &( status->route_data.edges[ landmark_index + 1 ] ); 

		// Is the next node in the edge the reverse of the current landmark?
		if ( landmark->reverse == edge->dest ){
			is_reverse = 1; 
			break; 
		}

		total_straight_distance = edge->dist; 
		landmark_index++; 
	}

	// Add the offset from the last landmark
	if ( is_reverse ){
		total_straight_distance += REVERSE_DEFAULT_OFFSET; 
	}
	else{
		// The offset is the offset of the goal. 
		total_straight_distance += status->current_goal.offset; 
	}

	*requires_reverse = is_reverse; 
	return total_straight_distance; 
}

/*
  This method determines which speed to use to reach a particular position. 
  Return: 
  - The best speed to use to reach a requested location. It also says if it is a "short" or
    "long" distance. 
*/
Train_speed calculate_speed_to_use( Train_status *status, Calibration_data *calibration_data ){
	// Initialization
	int speed_to_use, index, speed_found; 
	Train_speed train_speed; 
	Speed_calibration_data *speed_data; 

	// Calculate the speed required to travel the total straight distance. 
	int requires_reverse = 0; 
	int total_straight_distance = get_straight_distance( status, &requires_reverse );
	if ( total_straight_distance <= 0 ){
		// The train doesn't have to move. 
		train_speed.speed_type = SHORT_DISTANCE_SPEED; 
		train_speed.train_speed = 0;  
	}

	// Set the distance that this speed will cover. 
	train_speed.distance_to_travel = total_straight_distance;
	train_speed.requires_reverse = requires_reverse; 

	// Does the train require short speed? 
	if ( total_straight_distance <= calibration_data->short_speed_cutoff ){

		// Which short distance should we use? 
		train_speed.speed_type = SHORT_DISTANCE_SPEED; 
		speed_to_use = MAX_SPEED; 
		speed_found = 0; 

		while( speed_to_use > 0 && !speed_found ){
			// Iterate from the highest speed to the lowest
			speed_data = &( calibration_data->speed_data[ --speed_to_use ] ); 
			
			for ( index = 0; index < NUM_CALIBRATED_DISTANCES; index++ ){
				// This speed is too high; better use a lower one
				if ( index == 0 && speed_data->calibrated_distances[index] > total_straight_distance )
					break;

				if ( speed_data->calibrated_distances[index] > total_straight_distance || 
						speed_data->calibrated_distances[index] == 0  ){
					// We found a close speed that travels a longer distance than what we expected; we need to use
					// the previous one. 
					speed_found = 1; 
					break; 
				}
				else if ( speed_data->calibrated_distances[index] == total_straight_distance ){
					// A perfect match was found. 
					speed_found = 2; 
				}
			}

			if ( index == NUM_CALIBRATED_DISTANCES || speed_found ){
				if ( speed_found != 2 )
					index--;

				// TODO: If the distance is not completely precise, should we calculate an interpolation? 
				train_speed.train_speed = speed_to_use + 1; 
				train_speed.total_time_in_speed = speed_data->calibrated_distances_time[index]; 
				train_speed.time_accelerating = speed_data->calibrated_distances_time_to_deacc[index];
			}
		}
	}
	else{
		train_speed.speed_type = LONG_DISTANCE_SPEED;
		speed_to_use = MAX_SPEED; 
		speed_found = 0;

		while( speed_to_use > 0 && !speed_found ){
			// Iterate from the highest speed to the lowest
			speed_data = &( calibration_data->speed_data[ --speed_to_use ] ); 

			if ( speed_data->stopping_distance + speed_data->distance_during_acceleration < total_straight_distance )
				speed_found = 1; 
		}

		bwassert( train_speed.train_speed > 0, 
			"TRAIN_HELPERS: calculate_speed_to_use -> The train must move; the speed cannot be less than 1." ); 
		train_speed.train_speed = speed_to_use + 1; 
	}

	return train_speed; 
}

// -------------------------------------------------------------------
// Landmark Methods
// -------------------------------------------------------------------
void init_route(){
}

void update_train_position( int distance_since_update, Train_status *train_status, int *servers_list ){
	bwassert( distance_since_update >= 0, 
		"TRAIN_HELPERS: update_train_position -> The distance cannot be negative." ); 

	// First, calculate the distance after the previous landmark
	int distance_from_landmark = train_status->current_position.offset + distance_since_update; 

	// Second, calculate how far away is the train from the next landmark
	int distance_to_landmark = train_status->current_position.edge->dist; 
	if ( distance_from_landmark ){
		// The train hasn't reached a new landmark
		train_status->current_position.offset = distance_from_landmark; 
	}
	else{
		// The train has reached a new landmark
		train_status->current_position.landmark = train_status->current_position.edge->dest; 

		// Which type of landmark is it? 
		if ( train_status->current_position.landmark->type == NODE_BRANCH ){
			// The new node is a switch. 
			char temp_str[4]; 
			int switch_id = atoi( substr( temp_str, train_status->current_position.landmark->name, 2, 3 ) );

			// Which direction did the train take?
			Cmd_request cmd_request;
			cmd_request.type = QUERY_CMD_REQUEST; 
			cmd_request.cmd.cmd_type = SWITCH_STATE_CMD_TYPE; 
			cmd_request.cmd.element_id = switch_id; 

			Switch_query_reply query_msg_reply; 
			int sw_server_tid = servers_list[TH_SWITCH_SERVER_TID_INDEX]; 

			Send( sw_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 
				( char * ) &query_msg_reply, sizeof( query_msg_reply ) );

			if ( query_msg_reply.switch_position == SWITCH_STRAIGHT_POS ){
				train_status->current_position.edge = &( train_status->current_position.landmark->edge[DIR_STRAIGHT] );
			}
			else if ( query_msg_reply.switch_position == SWITCH_CURVE_POS ){
				train_status->current_position.edge = &( train_status->current_position.landmark->edge[DIR_CURVED] );
			}
			else {
				bwassert( 0, "TRAIN_HELPERS: update_train_position -> Invalid switch position." ); 
			}
		}
		else {
			train_status->current_position.edge = &( train_status->current_position.landmark->edge[DIR_AHEAD] );
		}

		if( train_status->goal_state == FIXED_GOAL ){
			// There's a goal, so update it's landmark index. 
			train_status->route_data.landmark_index++;
			bwassert( train_status->route_data.landmark_index < train_status->route_data.num_landmarks, 
				"TRAIN_HELPERS: update_train_position -> The landmark index must be less than the num of landmarks." ); 
		}

		train_status->current_position.offset = distance_from_landmark - distance_to_landmark; 
	}
}

float get_calibration_error(){
	return 0.0; 
}

// -------------------------------------------------------------------
// Route Methods
// -------------------------------------------------------------------
int request_new_path( Train_status *train_status, int *servers_list, track_node *track ){

	// 1. Get the current state of all sensors
	int switch_server_tid = servers_list[TH_SWITCH_SERVER_TID_INDEX];
	Cmd_request cmd_request;
	cmd_request.type = QUERY_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = ALL_SWITCHES_STATE_CMD_TYPE;
	cmd_request.cmd.element_id = 0;
	cmd_request.cmd.param = ( int ) &( train_status->route_data.switches_state ); // CAREFUL!!!
	Send( switch_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  );

	// 2. Send a request to the route server. 
	int route_server_tid = servers_list[TH_SWITCH_SERVER_TID_INDEX];
	char temp_switches_data[NUM_SWITCHES]; 

	// ---> Message to send to the server
	Route_msg route_msg; 
	route_msg.type = 0; 
	route_msg.track = track; 
	route_msg.train_direction = &( train_status->direction );
	route_msg.train_node = train_status->current_position.landmark; 
	route_msg.train_shift = train_status->current_position.offset; 
	route_msg.target_node = train_status->current_goal.landmark; 
	route_msg.target_shift = train_status->current_goal.offset; 
	route_msg.current_switches = train_status->route_data.switches_state; 
	route_msg.target_switches = temp_switches_data; 
	route_msg.landmarks = train_status->route_data.landmarks; 
	route_msg.edges = train_status->route_data.edges; 
	route_msg.num_landmarks = &( train_status->route_data.num_landmarks );
	Send( switch_server_tid, ( char * ) &route_msg, sizeof( route_msg ), 0, 0  );
			
	// 3. Update the goal information with the new path. 
	int i; 
	train_status->route_data.landmark_index = 0; 
	for( i = 0; i < NUM_SWITCHES; i++ ){
		train_status->route_data.switches_state[i] = temp_switches_data[i];
	}

	return 0; 
}

/*
  This method is used to update the landmark used as reference for the current position. 
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
