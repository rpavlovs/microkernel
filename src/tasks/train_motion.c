#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Command Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------------
void send_train_move_command( int tr_speed, int delay_time, Train_status *train_status, Train_server_data *server_data ){
	// First, get the command server tid
	int cmd_server_tid = server_data->tasks_tids[TR_CMD_SERVER_TID_INDEX]; 

	// If it's the same speed ignore this request. 
	if ( tr_speed == train_status->motion_data.train_speed )
		return;

	// Is velocity enabled?
	if ( tr_speed != TRAIN_STOP_CMD_SPEED && !server_data->calibration_data.speed_data[ tr_speed - 1 ].velocity_enabled )
		bwassert( 0, "TRAIN_MOTION: Velocity not enabled. [ speed: %d distance_type: %d ]", 
			tr_speed, train_status->motion_data.distance_type ); 

	// Send the command to the server
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Sending TR command. [ train_id: %d speed: %d delay_until: %d]", 
		train_status->train_id, tr_speed, delay_time );

	int current_time = TimeInMs(); 
	int new_time = 0; 
	if ( delay_time > 0 && delay_time > current_time ){
		// Prepare the message to send
		Train_cmd train_cmd; 
		train_cmd.delay_until = delay_time; 

		// Add the cmd that will be sent after the delay ends
		train_cmd.request.type = ADD_CMD_REQUEST; 
		train_cmd.request.cmd.cmd_type = TRAIN_CMD_TYPE;
		train_cmd.request.cmd.element_id = train_status->train_id;
		train_cmd.request.cmd.param = tr_speed; 

		// Push the result into the queue (if the notifier is sleeping it will be awaken at the end of this cycle)
		train_cmd_queue_push( train_cmd, &server_data->train_cmd_queue ); 
		new_time = delay_time; 
	}
	else{
		//bwprintf( COM2, "TRAIN_MOTION: Sending cmd [ Ticks: %d Ms: %d Train: %d Speed: %d ]\n", 
		//	current_time / 10, current_time, train_status->train_id, tr_speed ); 
		send_command( TRAIN_CMD_TYPE, train_status->train_id, tr_speed, cmd_server_tid );

		// TODO: Do we need to add a "CMD" delay. 
		new_time = current_time; 
	}
	//bwprintf( COM2, "SENDING TR [ TRAIN_ID: %d SPEED: %d DELAY: %d ]\n", train_status->train_id, tr_speed, delay_time ); 
	
	if ( tr_speed == TRAIN_STOP_CMD_SPEED ){
		train_status->motion_state = TRAIN_DEACCELERATING; 
		train_status->motion_data.time_since_deacceleration = new_time; 	
	}
	else {
		train_status->motion_state = TRAIN_ACCELERATING; 
		train_status->motion_data.train_speed = tr_speed;
		train_status->distance_since_speed_change = 0;
		train_status->time_speed_change = new_time;
	}
}

/*
  This method send the command server the reverse command and changes the direction of the train.
  NOTE: 
  - This method assumes that the train is in STILL position.
*/
void send_reverse_command( Train_status *train_status, Train_server_data *server_data ){
	// First, get the command server tid
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Sending RV command. [ train_id: %d ]", 
		train_status->train_id );
	int cmd_server_tid = server_data->tasks_tids[TR_CMD_SERVER_TID_INDEX]; 

	// Change the direction
	if ( train_status->train_direction == TRAIN_DIRECTION_FORWARD )
		train_status->train_direction = TRAIN_DIRECTION_REVERSE; 
	else
		train_status->train_direction = TRAIN_DIRECTION_FORWARD;

	// Send the reverse cmd to the command server
	send_command( REVERSE_CMD_TYPE, train_status->train_id, CMD_PARAM_NOT_REQUIRED, cmd_server_tid );
	train_status->is_reversing = 0; 

	// Change the reference landmark
	reverse_current_position( train_status ); 
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Distance Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------------
/*
	This method is used to calculate the time required to travel a distance at constant speed. 
	NOTE: 
    - This method is only used for long distances.
	Returns: 
	- The time to travel the required distance. It's given in ms. 
*/
float get_time_for_distance( float distance_to_travel, int train_speed, Train_server_data *server_data ){ 
	float velocity; 
	float required_time = 0.0; 

	if ( train_speed > 0 ){
		velocity = server_data->calibration_data.speed_data[ train_speed - 1 ].velocity;
		bwassert( velocity > 0, "TRAIN_MOTION: The velocity cannot be 0. [Train_speed: %d]", train_speed ); 

		required_time = distance_to_travel / velocity;
	}
	
	return required_time; 
}

float get_distance_in_constant_speed( int time_in_constant_speed, int train_speed, Train_server_data *server_data ){
	float velocity;
	float distance_traveled = 0;

	if ( train_speed > 0 ){
		velocity = server_data->calibration_data.speed_data[ train_speed - 1 ].velocity;
		distance_traveled = velocity * time_in_constant_speed;
	}

	return distance_traveled;
}

/*
  This method returns the distance traveled since the last refresh. 
*/
int get_distance_traveled( int current_time, Train_status *train_status, Train_server_data *server_data ){

	// NOTE:
	// Because of the nature of our project we don't change between speeds -> The only 
	// change from speed 0 to another speed, and then, at some point, we go back to 0.
	int time_since_change;
	if ( train_status->motion_state != TRAIN_DEACCELERATING && train_status->motion_data.distance_type == LONG_DISTANCE )
		time_since_change = current_time - train_status->time_speed_change;
	else
		time_since_change = current_time - train_status->motion_data.time_since_deacceleration; 

	if ( time_since_change <= 0 ){
		return 0; 
	}

	int distance_type = train_status->motion_data.distance_type;
	int distance_traveled = 0; 

	if ( distance_type == SHORT_DISTANCE )
		distance_traveled = get_short_distance_traveled( time_since_change, train_status, &( server_data->calibration_data ) ); 
	else
		distance_traveled = get_long_distance_traveled( time_since_change, train_status, &( server_data->calibration_data ) ); 

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_SERVER: get_distance_traveled -> Distance calculated. [ train_id: %d time_since_change: %d distance: %d ]", 
		train_status->train_id, time_since_change, distance_traveled );

	return distance_traveled; 
}

int get_long_distance_traveled( int time_since_change, Train_status *train_status, Calibration_data *calibration_data ){
    // NOTE: 
    // - Do we need to give some sort of delay between when the command was issued and when it was executed?
    bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
            "TRAIN_SERVER: get_long_distance_traveled -> Calculating distance traveled. [ train_id: %d time_since_change: %d motion_state: %d ]", 
			train_status->train_id, time_since_change, train_status->motion_state );

    int distance_traveled = 0; 
	int motion_state = train_status->motion_state;
	Train_motion_data *motion_data = &train_status->motion_data;
	Speed_calibration_data *speed_calibration = &( calibration_data->speed_data[ motion_data->train_speed - 1 ] ); 

    if ( motion_state == TRAIN_ACCELERATING || motion_state == TRAIN_CONSTANT_SPEED ){
		// Has the train reached constant speed? 
		if ( motion_state == TRAIN_ACCELERATING && time_since_change >= speed_calibration->time_to_constant_speed ){
			train_status->motion_state = TRAIN_CONSTANT_SPEED; 
		}

        if ( train_status->motion_state == TRAIN_ACCELERATING ){
                // The train is accelerating, but the acceleration is not constant. However, since the 
                // distance is long we don't care about such small changes. We only care about the distance 
                // and time it will take to completely accelerate.
			distance_traveled = speed_calibration->distance_during_acceleration * time_since_change;
            distance_traveled /=  speed_calibration->time_to_constant_speed; 

			bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
				"TRAIN_SERVER: get_long_distance_traveled -> still accelerating. [ time_since_ch: %d time_to_speed: %d  dist: %d ]", 
				time_since_change, speed_calibration->time_to_constant_speed,  distance_traveled ); 
        }
        else {
                // The train has already reached constant speed. Therefore we need to sum the distance 
                // traveled during acceleration and the distance traveled on constant speed. 
                int time_constant_speed = time_since_change - speed_calibration->time_to_constant_speed;
                distance_traveled = speed_calibration->distance_during_acceleration; 
				distance_traveled += round_decimal_up( time_constant_speed * speed_calibration->velocity ); 

				bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
					"TRAIN_SERVER: get_long_distance_traveled -> Constant Speed. [ time_since_ch: %d time_to_speed: %d  dist: %d ]", 
					time_since_change, speed_calibration->time_to_constant_speed,  distance_traveled ); 
        }
    }
    else if ( motion_state == TRAIN_DEACCELERATING ){
        // Since we don't know the (de) acceleration rate, but only the stopping distance and stopping time.
        // Even though it's not completely accurate, we just make a linear interpolation. 
                
        // Previous speed
		if ( !train_status->distance_before_deacceleration ){
			int time_constant_speed = 
				train_status->motion_data.time_since_deacceleration - speed_calibration->time_to_constant_speed - train_status->time_speed_change;
			distance_traveled = speed_calibration->distance_during_acceleration; 
			distance_traveled += round_decimal_up( time_constant_speed * speed_calibration->velocity ); 
			train_status->distance_before_deacceleration = distance_traveled; 
			///*
			bwprintf( COM2, "\n--------- CALCULATING DISTANCE DURING CONSTANT SPEED: ---------------------------\n" ); 
			bwprintf( COM2, "   - Time_Deacc: %d Time_to_csp: %d Time_sp_ch: %d Time_CSP: %d\n", 
				train_status->motion_data.time_since_deacceleration, speed_calibration->time_to_constant_speed, 
				train_status->time_speed_change, time_constant_speed ); 
			bwprintf( COM2, "   - Dist_Acc: %d Dist_Trav: %d\n", speed_calibration->distance_during_acceleration, distance_traveled ); 
			///*
		}
		else {
			distance_traveled = train_status->distance_before_deacceleration;
		}

        // Speed during de-acceleration. 
        int time = speed_calibration->stopping_time -  time_since_change; 
        if ( time > 0 ){
			int distance = speed_calibration->stopping_distance * time_since_change; 
			distance /= speed_calibration->stopping_time; 
			distance_traveled += distance; 
        }
        else{
            // The train has already stopped. 
            distance_traveled += speed_calibration->stopping_distance; 
            train_status->motion_state = TRAIN_STILL; 
			train_status->motion_data.train_speed = TRAIN_STOP_CMD_SPEED;
        }
    }
    else {
        // Wrong state. 
        bwassert( 0, "TRAIN_MOTION: get_long_distance_traveled -> Wrong train state." );
    }

			bwprintf( COM2, "\n--------- CALCULATING DISTANCE DURING CONSTANT SPEED: ---------------------------\n" ); 
			bwprintf( COM2, "   - Dist_sp_ch: %d Dist_Trav: %d Distance_acc: %d State: %d Time SP: %d Time_to_CSP: %d Time_Deacc: %d \n", 
				train_status->distance_since_speed_change, distance_traveled, train_status->distance_before_deacceleration, train_status->motion_state, 
				train_status->time_speed_change, speed_calibration->time_to_constant_speed, train_status->motion_data.time_since_deacceleration ); 

    // distance_traveled contains the distance since the speed change. Now this distance is compared to the 
    // one calculated during the previous refresh. 
    // TODO: After checking the sensors remember to correctly update this parameters to avoid having negative values. 
    int distance_since_change = distance_traveled - train_status->distance_since_speed_change;
    bwassert( distance_since_change >= 0, 
		"TRAIN_MOTION: get_long_distance_traveled -> The distance cannot be negative. [ dist_traveled: %d dist_since_sp_ch: %d ]", 
		distance_traveled, train_status->distance_since_speed_change ); 

    // Update state
    train_status->distance_since_speed_change = distance_traveled;

    bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
            "TRAIN_MOTION: get_long_distance_traveled -> Distance traveled since update. [ train_id: %d distance: %d ]", 
            train_status->train_id, distance_traveled );

    return distance_since_change; 
}

int get_short_distance_traveled( int time_since_change, Train_status *train_status, Calibration_data *calibration_data ){

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: get_short_distance_traveled -> Calculating distance traveled. [ train_id: %d time_since_change: %d ]", 
		train_status->train_id, time_since_change );

	int distance_traveled = 0; 
	int motion_state = train_status->motion_state; 
	if ( motion_state == TRAIN_ACCELERATING || motion_state == TRAIN_CONSTANT_SPEED || motion_state == TRAIN_DEACCELERATING ){
		// The train is moving. We don't care about how long it takes to get to constant
		// speed if ever, or how long it takes to deaccelerate. The distance is very short
		// so we only work in terms of total distance and total speed. 
		int time, total_travel_time; 
		Train_motion_data *motion_data = &train_status->motion_data; 
		Speed_calibration_data *speed_calibration = &( calibration_data->speed_data[ motion_data->train_speed ] ); 

		total_travel_time = get_short_distance_total_time( motion_data->calibrated_dist_index, speed_calibration ); 
		time = total_travel_time - time_since_change;

		if ( time > 0 ){
			// Train is still moving. 
			distance_traveled = ( motion_data->distance_to_travel * time_since_change ) / ( total_travel_time );
		}
		else{
			// The train has already stopped
			distance_traveled = motion_data->distance_to_travel; 
			train_status->motion_state = TRAIN_STILL; 
		}
	}
    else {
		// Wrong state. 
		bwassert( 0, "TRAIN_MOTION: get_short_distance_traveled -> Wrong train state." );
	}

	// Get the distance traveled since last refresh
    int distance_since_change = distance_traveled - train_status->distance_since_speed_change;
    bwassert( distance_since_change >= 0, 
            "TRAIN_MOTION: get_short_distance_traveled -> The distance cannot be negative." ); 

    // Update state
    train_status->distance_since_speed_change = distance_traveled;

    bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
            "TRAIN_MOTION: get_short_distance_traveled -> Distance traveled since update. [ train_id: %d distance: %d ]", 
            train_status->train_id, distance_since_change );

    return distance_since_change; 
}

/*
  This method calculates the total straight distance that the train must travel while following it's
  route. A straight distance is defined as the distance that the train must travel without stopping. 
  The train might stop because of two reasons: it has reached its destination or it must reverse. 
  Return: 
  - The total straight distance. 
*/
int get_straight_distance( Train_status *status, Train_server_data *server_data ){
        // Initialization
        int is_reverse = 0; 
		int final_offset_dist = 0; 
        int total_straight_distance = 0; 
		track_edge *edge; 
        track_node *landmark;
		track_node *next_landmark = 0;
        int landmark_index = status->route_data.landmark_index; 
        int num_landmarks = status->route_data.num_landmarks; 

        bwassert( landmark_index < num_landmarks, 
                "TRAIN_MOTION: get_straight_distance -> The landmark index must be less than the num of landmarks." ); 

        // Remove the distance that the train has already moved from the initial landmark. 
        total_straight_distance -= status->current_position.offset; 
		//bwprintf( COM2, "Initial offset Dist: %d\n", total_straight_distance ); 

		landmark = status->route_data.landmarks[ landmark_index ]; 

        // Get the distance of the next landmarks without reverse (reverse would require the train to stop).
        // NOTE: Here we only count FULL edges; the final offset will be considered later.
        while( landmark_index + 1 < num_landmarks ){

			edge = status->route_data.edges[ landmark_index ];
            landmark = status->route_data.landmarks[ landmark_index ]; 
			next_landmark = status->route_data.landmarks[ landmark_index + 1 ];

			// Is there a reverse? 
			if ( next_landmark->reverse == landmark ){
				is_reverse = 1;
				break; 
			}
	
			//bwprintf( COM2, "Landmark: %s Dist: %d\n", landmark->name, edge->dist ); 

			total_straight_distance += edge->dist;
            landmark_index++; 
        }

		// Specify the current landmark (for debugging purposes)
		if ( next_landmark )
			landmark = next_landmark; 

        // Add the offset from the last landmark
		if ( is_reverse && next_landmark != status->current_goal.landmark ){
			final_offset_dist = get_sw_clear_movement_distance( status, server_data );
        }
		else if( !is_reverse ){
			// The offset is the offset of the goal. 
			// NOTE: This offset is only added if we need to travel in a straight line to the goal
			final_offset_dist = status->current_goal.offset;
        }
		else{
			// This route requires a reverse to get to the goal. However, this reverse is on the final
			// landmark; no extra space needs to be added. 
		}
		//bwprintf( COM2, "Final offset Dist: %d\n", final_offset_dist );
		total_straight_distance += final_offset_dist; 

		// In case the only landmarks were for a reverse, then the distance to travel is 0. 
		if ( total_straight_distance < 0 )
			total_straight_distance = 0; 

		/*
		bwdebug( DBG_USR, TEMP2_DEBUG_AREA, 
			"TRAIN_MOTION: \nget_straight_distance -> Landmark1: %s Offset: %d Landmark2: %s Offset: %d Dist: %d\n", 
			status->current_position.landmark->name, status->current_position.offset, 
			landmark->name, final_offset_dist, total_straight_distance ); */
		bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
			"TRAIN_MOTION: \nget_straight_distance -> Landmark1: %s Offset: %d Landmark2: %s Offset: %d Dist: %d\n", 
			status->current_position.landmark->name, status->current_position.offset, 
			landmark->name, final_offset_dist, total_straight_distance ); 

        return total_straight_distance; //+ 1; // TODO: Should we keep this +1?
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Speed Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------------
/*
  This method determines which speed to use to reach a particular position. 
  Return: 
  - The best speed to use to reach a requested location. It also says if it is a "short" or
    "long" distance. 
*/
int calculate_speed_to_use( Train_status *status, Train_server_data *server_data ){

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: Starting calculation of speed to use. [ train_id: %d ]", status->train_id );

	// Initialization
	int speed_to_use, index, speed_found, distance_in_speed; 
	Speed_calibration_data *speed_data; 
	Calibration_data *calibration_data  = &server_data->calibration_data; 

    // Calculate the speed required to travel the total straight distance. 
    int total_straight_distance = get_straight_distance( status, server_data );
    if ( total_straight_distance <= 0 ){
		// The train doesn't have to move. 
		return 0; 
    }

	bwdebug( DBG_USR, TEMP_DEBUG_AREA, "TOTAL_STRAIGHT_DISTANCE: %d", total_straight_distance ); 

    // Set the distance that this speed will cover. 
	status->motion_data.distance_to_travel = total_straight_distance; 

    // Does the train require short speed? 
    if ( total_straight_distance <= calibration_data->short_speed_cutoff ){

        // Which short distance should we use? 
		//bwprintf( COM2, "Calculating short distance [ dist: %d ] \n", status->motion_data.distance_to_travel );
		status->motion_data.distance_type = SHORT_DISTANCE; 
        speed_to_use = 6; 
        speed_found = 0; 

		// We only consider speed 6
		speed_to_use--; 
		speed_data = &( calibration_data->speed_data[ speed_to_use ] );

		for( index = 0; index < NUM_CALIBRATED_DISTANCES; index++ ){
			distance_in_speed = get_short_distance( index, speed_data ); 

			if ( distance_in_speed >= status->motion_data.distance_to_travel ){
				speed_found = 1; 
				break; 
			}
		}

		bwprintf( COM2, "Speed for short distance found [ speed: %d index: %d dist: %d speed_cutoff: %d ] \n", 
			speed_to_use + 1, index, total_straight_distance, calibration_data->short_speed_cutoff );
		bwassert( speed_found, "TRAIN_MOTION: calculate_speed_to_use -> Short speed must always be found. Maybe the the speed cutoff is wrong.\n"
			" [ distance: %d short_sp_cutoff: %d ]", total_straight_distance, calibration_data->short_speed_cutoff ); 

		//bwprintf( COM2, "Speed for short distance found [ speed: %d index: %d ] \n", speed_to_use + 1, index );
		// Set the index of the calibrated distance that will be used
		status->motion_data.calibrated_dist_index = index; 
    }
    else{
		status->motion_data.distance_type = LONG_DISTANCE;
        speed_to_use = MAX_SPEED; 
        speed_found = 0;

		int distance_to_travel; 
        while( speed_to_use > 0 && !speed_found ){
                // Iterate from the highest speed to the lowest
                speed_data = &( calibration_data->speed_data[ --speed_to_use ] ); 

				distance_to_travel = speed_data->stopping_distance + speed_data->distance_during_acceleration;
				if ( speed_data->velocity_enabled && distance_to_travel < total_straight_distance )
                        speed_found = 1; 
        }

		bwassert( speed_to_use > 0, 
                "TRAIN_MOTION: calculate_speed_to_use -> The train must move; the speed cannot be less than 1.\n"
				"[ distance: %d short_sp_cutoff: %d ]", total_straight_distance, calibration_data->short_speed_cutoff ); 
    }

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: \ncalculate_speed_to_use -> Total_Dist %d Speed_to_use: %d Type: %d \n", 
			total_straight_distance, speed_to_use + 1, status->motion_data.distance_type );

	return speed_to_use + 1; 
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Landmark Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------------
int has_train_arrived( Train_status *train_status, Train_server_data *server_data ){
	if ( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
        Train_position current_position = train_status->current_position; 
        Train_position goal_position = train_status->current_goal; 
        if (
			current_position.landmark == goal_position.landmark && 
			( current_position.offset >= ( goal_position.offset - 2 ) )  )
		{
			// TODO: Uncomment this to enable the adjustment of the location based on the error. 
			/*
			if ( train_status->motion_data.current_error > 0 ){
				
				//adjust_pos_with_sensor_data( train_status, server_data );
				clear_train_motion_data( train_status ); 
			}
			*/
			//bwprintf( COM2, "ARRIVED! Total Distance: %d\n", train_status->distance_since_speed_change ); 
			return 1; 
		}
    }
    return 0; 
}

void update_train_position_landmark( int distance_since_update, Train_status *train_status, int *servers_list ){
    bwassert( distance_since_update >= 0, 
            "TRAIN_MOTION: update_train_position -> The distance cannot be negative." ); 

	// 1. Get the current landmark and edge
	track_node *current_node = train_status->current_position.landmark; 
	track_edge *current_edge = train_status->current_position.edge; 

	// 2. Calculate the distance from the current landmark. 
	int distance_from_landmark = train_status->current_position.offset + distance_since_update; 

	// 3. Calculate the distance to the next landmark. 
	int distance_to_landmark = current_edge->dist; 

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
        "TRAIN_MOTION: update_train_position_landmark -> "
		"Calculating new landmark and offset. [ train_id: %d landmark: %s current_pos: %d distance_from: %d distance_to: %d ]", 
		train_status->train_id, current_node->name, train_status->current_position.offset,
		distance_from_landmark, distance_to_landmark );

	/*
	bwprintf( COM2, 
		"TRAIN_MOTION: update_train_position_landmark -> \n"
		"Calculating new landmark and offset. \n[ train_id: %d landmark: %s current_pos: %d distance_from: %d distance_to: %d tot_dist: %d ]\n", 
		train_status->train_id, current_node->name, train_status->current_position.offset,
		distance_from_landmark, distance_to_landmark, train_status->distance_since_speed_change );
		*/
	while( ( distance_from_landmark - distance_to_landmark ) >= 0 ){

		// The train has reached a new landmark. 
		current_node = current_edge->dest; 

		// Which type of landmark is it? 
		int landmark_type = current_node->type;
		if ( landmark_type == NODE_BRANCH ){
			// The new node is a switch. 
			char temp_str[6]; 
			int switch_id = atoi( substr( temp_str, current_node->name, 2, 6 ) );

            // Which direction did the train take?
            Cmd_request cmd_request;
            cmd_request.type = QUERY_CMD_REQUEST; 
            cmd_request.cmd.cmd_type = SWITCH_STATE_CMD_TYPE; 
            cmd_request.cmd.element_id = switch_id; 

            Switch_query_reply query_msg_reply; 
            int sw_server_tid = servers_list[TR_SWITCH_SERVER_TID_INDEX]; 

            Send( sw_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 
                    ( char * ) &query_msg_reply, sizeof( query_msg_reply ) );

			if ( query_msg_reply.switch_position == SWITCH_STRAIGHT_POS ){
				current_edge = &( current_node->edge[ DIR_STRAIGHT ] ); 
			}
			else if ( query_msg_reply.switch_position == SWITCH_CURVE_POS ){
				current_edge = &( current_node->edge[ DIR_CURVED ]); 
			}
			else{
                bwassert( 0, 
					"TRAIN_MOTION: update_train_position -> Invalid switch position [ sw_id: %d pos: %c ]", 
					query_msg_reply.switch_id, query_msg_reply.switch_position );
			}
		}
		else{
			current_edge = &( current_node->edge[ DIR_AHEAD ] );
		}

		// If there's a goal update its landmark index. 
		if( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
			int landmark_index = train_status->route_data.landmark_index;
			if ( landmark_index < train_status->route_data.num_landmarks ){
				if ( train_status->route_data.landmarks[ landmark_index + 1 ] == current_node )
					train_status->route_data.landmark_index++;
			}
        }

		// Update to the new values
		distance_from_landmark -= distance_to_landmark; 
		distance_to_landmark = current_edge->dist; 

		// Make sure the movement is not outside normal boundaries. 
		if ( current_node->type == NODE_EXIT && distance_from_landmark > 0 ){
			bwassert( 0, "TRAIN_MOTION: update_train_position_landmark -> Invalid node [ node: %s distance_since_update: %d ]", 
				current_node->name, distance_from_landmark ); 
		}
	}

	// Update the train status with the calculated values
	train_status->current_position.edge = current_edge; 
	train_status->current_position.landmark = current_node; 
	train_status->current_position.offset = distance_from_landmark; 
	/*
	bwprintf( COM2, 
		"TRAIN_MOTION: update_train_position_landmark PATH CALCULATED -> \n"
		"[ train_id: %d landmark: %s current_pos: %d ]\n", 
		train_status->train_id, train_status->current_position.landmark->name,
		train_status->current_position.offset );
		*/
}

void update_train_position_neg_landmark( int dist_to_adjust, Train_status *train_status, Train_server_data *server_data ){
	bwassert( dist_to_adjust >= 0, 
		"TRAIN_MOTION: update_train_position_neg_landmark -> The distance cannot be negative." ); 

	// 1. Get the current landmark and edge. 
	track_node *current_node = train_status->current_position.landmark; 
	track_edge *current_edge = train_status->current_position.edge; 

	// 2. Calculate the distance from the current landmark
	int distance_from_landmark = get_abs_value( train_status->current_position.offset - dist_to_adjust ); 

	// 3. Calculate the distance to the next landmark
	int distance_to_landmark = train_status->current_position.offset; 

	while( ( distance_from_landmark - distance_to_landmark ) >= 0 ){
		// The train has reached a new landmark. 
		current_node = current_edge->reverse->dest->reverse; 

		// Which type of landmark is it? 
		int landmark_type = current_node->type; 
		if ( landmark_type == NODE_BRANCH ){
			char curr_sw_pos = get_current_sw_pos( current_node, train_status, server_data ); 

			if ( curr_sw_pos == SWITCH_STRAIGHT_POS ){
				current_edge = current_node->reverse->edge[ DIR_STRAIGHT  ].reverse; 
			}
			else if ( curr_sw_pos == SWITCH_CURVE_POS ){
				current_edge = current_node->reverse->edge[ DIR_CURVED ].reverse; 
			}
			else{
				bwassert( 0, 
					"TRAIN_MOTION: update_train_position_neg_landmark -> Invalid switch position [ sw_id: %s pos: %c ]", 
					current_node->name, curr_sw_pos );
			}
		}
		else{
			current_edge = current_node->reverse->edge[ DIR_STRAIGHT ].reverse;
		}

		// Update to the new values
		distance_from_landmark -= distance_to_landmark; 
		distance_to_landmark = current_edge->dist; 

		// Make sure the movement is not outside normal boundaries. 
		if ( current_node->type == NODE_ENTER && distance_from_landmark > 0 ){
			bwassert( 0, "TRAIN_MOTION: update_train_position_neg_landmark -> Invalid node [ node: %s dist_to_adj_left: %d ]", 
				current_node->name, distance_from_landmark ); 
		}
	}

	// Update the train status with the calculated values
	train_status->current_position.edge = current_edge; 
	train_status->current_position.landmark = current_node; 
	train_status->current_position.offset = distance_from_landmark; 
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Train Motion
// -----------------------------------------------------------------------------------------------------------------------------------------------------
void update_train_status( Train_update_request *update_request, Train_status *train_status, Train_server_data *server_data ){
	
	// Initialization
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: Starting train update. [ update_type: %d ]", update_request->update_type );

	// TODO: Get this time at the beginnging of the server's operation; that way it will be more accurate
	int current_time = TimeInMs();

	// 1. Calculate the current position of the train. 
	update_train_position( current_time, train_status, server_data ); 

	// 2. Handle the update request
	handle_update_request( update_request, train_status, server_data ); 

	if ( update_request->update_type != UPDATE_FROM_SENSOR ){
		// 3. Calculate the behavior of the train before the next update and 
		//    determine if a command needs to be sent. 
		predict_train_movement( current_time, train_status, server_data );

		// 4. Print the information. 
		print_train_status( train_status ); 

		// 5. Update the status in the screen
		send_dashboard_train_pos( train_status, server_data );
	}
}

void update_train_position( int current_time, Train_status *train_status, Train_server_data *server_data ){

	if ( train_status->motion_state == TRAIN_STILL ){
		// The train is still and the speed hasn't changed, so nothing to do here.
		bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
			"TRAIN_MOTION: update_train_status -> Train is not moving; there's nothing to update. [ train_id: %d ]", train_status->train_id );
		return; 
	}

	// Initialization
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: update_train_status -> Starting train status update. [ train_id: %d ]", train_status->train_id );

	// Get the distance that the train has moved since the last update. 
	int distance_traveled = get_distance_traveled( current_time, train_status, server_data ); 

	// Get the distance from the last landmark.
	update_train_position_landmark( distance_traveled, train_status, server_data->tasks_tids );
}

void handle_update_request( Train_update_request *update_request, Train_status *train_status, Train_server_data *server_data ){
	// What type of request is this? 
	if ( update_request->update_type == MOVE_FREE_UPDATE ){
		// The train has been ordered to move freely. 
		// If there was a previous goal, remove it. 
		initialize_goal( train_status ); 

		// TODO: Make the sensor detective check all sensors -> We are assuming that there's only 1 train in the track. 

		// Send the commands
		int cmd_type = update_request->cmd.cmd_type; 
		switch( cmd_type ){
			case TRAIN_CMD_TYPE:
				train_status->motion_data.original_train_speed = 0;
				train_status->motion_data.distance_type = LONG_DISTANCE; 
				send_train_move_command( update_request->cmd.param, 0, train_status, server_data ); 
				break; 
			case REVERSE_CMD_TYPE:
				train_status->motion_data.original_train_speed = train_status->motion_data.train_speed; 
				train_status->motion_data.distance_type = LONG_DISTANCE; 
				send_train_move_command( TRAIN_STOP_CMD_SPEED, 0, train_status, server_data );
				train_status->is_reversing = 1; 
				break; 
			default:
				// Should never get here
				bwassert( 0, "TRAIN_MOTION: update_train_status -> Invalid command type." );
				break; 
		}
	}
	else if ( update_request->update_type == CHANGE_GOAL_UPDATE ){
		// There's a new goal. 
		// 1. Erase previous goal information 
		initialize_goal( train_status ); 

		// 2. Get a new path
		// NOTE: For the edge we always consider straight line. If the user wants to move to a curve branch, 
		// then he needs to give another landmark. 
		train_status->current_goal.landmark = ( track_node * ) update_request->cmd.element_id;	// CAREFUL!!!
		train_status->current_goal.offset = update_request->cmd.param;
		train_status->current_goal.edge = &( train_status->current_goal.landmark->edge[ DIR_AHEAD ] ); 

		int path_found = request_new_path( train_status, server_data );
		bwassert( path_found, "TRAIN_MOTION: Path couldn't be found [ train_id: %d curr_landmark: %s curr_offset: %d dest: %s ]", 
			train_status->train_id, train_status->current_position.landmark->name, train_status->current_position.offset, 
			train_status->current_goal.landmark->name ); 

		// 3. Now the train has a goal. Update the status accordingly.
		train_status->train_state = TRAIN_STATE_MOVE_TO_GOAL; 
	}
	else if ( update_request->update_type == UPDATE_FROM_SENSOR ){
		update_with_sensor_data( update_request->triggered_sensor, train_status, server_data ); 
	}
}

void start_short_distance_movement( int speed_to_use, Train_status *train_status, Train_server_data *server_data ){
	// Variables
	int start_cmd_delay, stop_cmd_delay, distance_to_reserve, is_dist_reserved; 
	int num_sw_changed, default_time_to_stop, default_distance_traveled; 

	// Reserve the distance to use
	distance_to_reserve = train_status->motion_data.distance_to_travel;
	is_dist_reserved = reserve_distance( distance_to_reserve, train_status, server_data ); 

	if ( is_dist_reserved ){
		// TODO: Do we need to update the sensor attribution list here? 

		// Do we need to move any switches? 
		start_cmd_delay = 0; 
		num_sw_changed = check_next_sw_pos( distance_to_reserve, train_status, server_data );
		if ( num_sw_changed ){
			//cmd_delay = current_time + get_sw_delay_time( server_data ); 
		}

		// Determine when to stop the train
		Calibration_data *calibration_data = &server_data->calibration_data;
		Speed_calibration_data *speed_calibration =  &( calibration_data->speed_data[ speed_to_use - 1 ] ); 
		int calibrated_dist_index = train_status->motion_data.calibrated_dist_index;
		default_time_to_stop = get_short_distance_stopping_time( calibrated_dist_index, speed_calibration );
		default_distance_traveled = get_short_distance( calibrated_dist_index, speed_calibration );

		start_cmd_delay += TimeInMs(); // Current time
		stop_cmd_delay = ( distance_to_reserve * default_time_to_stop ) / default_distance_traveled;
		stop_cmd_delay += start_cmd_delay;

		// Send the commands
		send_train_move_command( speed_to_use, start_cmd_delay, train_status, server_data );
		send_train_move_command( TRAIN_STOP_CMD_SPEED, stop_cmd_delay, train_status, server_data ); 
	}
	else{
		// Couldn't move -> reset. 
		clear_train_motion_data( train_status ); 
	}
}

int track_train_short_distance( Train_status *train_status, Train_server_data *server_data ){
	int continue_execution = 0; 
	if ( train_status->motion_state == TRAIN_STILL ){
		// For short distances the only important state is when the train is still. 
		// We don't care about the other states because the distances are too small. 

		// TODO: Is this correct? Shouldn't we just clean the motion data and let the other method
		// determine what to do? 
		if ( has_train_arrived( train_status, server_data ) ){
			// The train has reached its destination
			// - Remove the current goal
			initialize_goal( train_status );

			continue_execution = 1; 
		}

		// Release all the reserved track; keep only the current size of the train. 
		reserve_distance( 0, train_status, server_data );
	}

	return continue_execution;
}

void predict_train_movement( int current_time, Train_status *train_status, Train_server_data *server_data ){
	// Initialization
	int cmd_delay, speed_to_use, distance_to_reserve, time_in_constant_speed; 
	int is_distance_reserved, next_update_time, accurate_current_time ; 

	// If the train is moving a short distance track it somewhere else
	if ( train_status->motion_data.distance_type == SHORT_DISTANCE ){
		int continue_exec = track_train_short_distance( train_status, server_data );
		if ( !continue_exec )
			return; 
	}

	// Is the train supposed to move?
	// - If not, there's no need to stay in this method. 
	if ( train_status->train_state == TRAIN_STATE_MOVE_FREE ){
		int original_speed = train_status->motion_data.original_train_speed; 
		int curr_speed = train_status->motion_data.train_speed; 

		if ( original_speed == curr_speed && curr_speed == 0 )
			return;
	}

	// Determine what's going to happen in the future, and how to react to it. 
	switch( train_status->motion_state ){
		case TRAIN_STILL:
			// -- Initialization --
			accurate_current_time = TimeInMs(); 
			cmd_delay = accurate_current_time; 

			// 1. DETERMINE DIRECTION
			//		- Is the train facing at the right direction?
			if ( train_status->train_state == TRAIN_STATE_MOVE_FREE ){
				if ( train_status->is_reversing ){
					// TODO: Do we need to add a delay because of this movement? 
					send_reverse_command( train_status, server_data );
				}

				speed_to_use = train_status->motion_data.original_train_speed;
			}
			else if ( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
				if ( has_train_arrived( train_status, server_data ) ){
					// The train has reached its destination
					// - Remove the current goal
					initialize_goal( train_status );

					// Release all track reserved ( will only keep the current position of the train ).
					reserve_distance( 0, train_status, server_data ); 
					add_sensors_attrib_list( train_status, server_data );
					break; 
				}

				// Does the train need to reverse? 
				if ( requires_reversing( train_status ) ){
					train_status->is_reversing = 1; 
					cmd_delay += get_reverse_delay_time( server_data ); 
					send_reverse_command( train_status, server_data );

					// NOTE: We calculate the route again in case the "extra" space
					// used for reverse causes problems. 
					clear_train_motion_data( train_status );
					int path_found = request_new_path( train_status, server_data );
					bwassert( path_found, "TRAIN_MOTION: Path couldn't be found" );
					reserve_distance( 0, train_status, server_data );
				}

				// Calculate the next "straight" movement
				speed_to_use = calculate_speed_to_use( train_status, server_data );
			}

			if ( train_status->motion_data.distance_type == SHORT_DISTANCE ){
				// Since the behavior of short distances is different, they are handled in a different function. 
				start_short_distance_movement( speed_to_use, train_status, server_data );
			}
			else{
				// 2. RESERVATION
				//		- The train is going to move, and it's already facing the right direction. 
				//		- How far should we reserve?
				distance_to_reserve = 
					get_train_acceleration_distance( speed_to_use, server_data ) + 
					get_train_stopping_distance( speed_to_use, server_data ) + 
					get_reservation_distance_buffer( server_data );

				is_distance_reserved = reserve_distance( distance_to_reserve, train_status, server_data ); 

				// 3. START MOVEMENT
				//		- If the reservation was successful, are there any switches that need to be moved? 
				//		- If the reservation was successful, start moving
				if ( is_distance_reserved ){
					if ( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
						// Move switches within that distance
						int num_sw_changed = check_next_sw_pos( distance_to_reserve, train_status, server_data );
						if ( num_sw_changed ){
							//cmd_delay = current_time + get_sw_delay_time( server_data ); 
						}

						// Add the sensors to attribution list.
						add_sensors_attrib_list( train_status, server_data ); 
					}

					// Start moving
					//bwprintf( COM2, "Starting Moving: [ train_id: %d speed: %d dir: %d ]\n", 
					//	train_status->train_id, speed_to_use, train_status->train_direction ); 
					send_train_move_command( speed_to_use, cmd_delay, train_status, server_data );
				}
			}

			break; 
		case TRAIN_ACCELERATING:
		case TRAIN_CONSTANT_SPEED:
			// When will the next update be done? 
			next_update_time = get_next_update_time( current_time );

			// 1. Calculate distance to reserve
			// -- Get the basic data
			int time_speed_change = train_status->time_speed_change;
			int time_to_reach_constant_sp = get_time_to_constant_speed( train_status, server_data );
			int time_to_start_deacceleration = get_time_to_start_deaccelerating( train_status, server_data ); 
			time_in_constant_speed = time_to_start_deacceleration - time_to_reach_constant_sp; 
			
			// -- The distance to reserve regardless of the amount of constant data traveled
			distance_to_reserve = 
				get_train_stopping_distance( train_status->motion_data.train_speed, server_data ) + 
				get_reservation_distance_buffer( server_data );  

			if ( time_speed_change + time_to_reach_constant_sp < next_update_time ){
				
				// The train will move some time at constant speed.
				// -- How much time will the train move at constant speed before the next update? 
				int init_time, end_time, time_const_speed_before_update; 
				//int time_const_speed_before_update;
				if ( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){

					// Check when the constant speed was reached in this update interval
					if( time_speed_change + time_to_reach_constant_sp <= current_time )
						init_time = current_time; 
					else
						init_time = time_speed_change + time_to_reach_constant_sp; 

					// Check when to stop counting constant speed in this update interval
					if ( time_speed_change + time_to_start_deacceleration < next_update_time )
						end_time = time_speed_change + time_to_start_deacceleration;
					else
						end_time = next_update_time;

					time_const_speed_before_update = end_time - init_time; 
				}
				else{
					time_const_speed_before_update = current_time - next_update_time; 
				}

				// -- The distance traveled during constant speed before the next update also needs to be reserved. 
				int train_speed = train_status->motion_data.train_speed;
				int distance_constant_speed = round_decimal_up( get_distance_in_constant_speed( time_const_speed_before_update, train_speed, server_data ) );
				distance_to_reserve += distance_constant_speed;

				bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_MOTION: Train traveled at constant speed. [ train_id: %d distance_traveled: %d ]", 
					train_status->train_id, distance_constant_speed );
			}

			// 2. Reserve the distance
			//bwprintf( COM2, "[ RESERVATION -> Reserved_distance: %d ]\n", distance_to_reserve );
			is_distance_reserved = reserve_distance( distance_to_reserve, train_status, server_data ); 
			
			// 3. Prepare everything in the reserved distance for the train movement
			if ( is_distance_reserved ){
				
				// Check if there are switches to move in the reserved distance. 
				check_next_sw_pos( distance_to_reserve, train_status, server_data );

				// Does that train need to start de-accelerating?
				int temp_time = time_speed_change + time_to_start_deacceleration; 
				////bwprintf( COM2, "DEACCELERATE -> CURRENT_TIME: %d NEXT_UPD_TIME: %d TIME_SP_CHANGE: %d TIME_TO_DEACC: %d\n", 
				//	current_time, next_update_time, time_speed_change, time_to_start_deacceleration );
				if ( temp_time >= current_time && temp_time < next_update_time ){
					send_train_move_command( TRAIN_STOP_CMD_SPEED, temp_time, train_status, server_data );
				}

				// Add sensors to attribution list
				add_sensors_attrib_list( train_status, server_data ); 
			}
			else{
				// The new distance couldn't be reserved. Stop as soon as we reach constant speed!!!
				bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_MOTION: Couldn't reserve distance. Stopping!! [ train_id: %d distance_requested: %d ]", 
					train_status->train_id, distance_to_reserve ); 
				send_train_move_command( TRAIN_STOP_CMD_SPEED, time_speed_change + time_to_reach_constant_sp, train_status, server_data );
			}

			break;
		case TRAIN_DEACCELERATING:
			bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_MOTION: Deaccelerating. [ train_id: %d ]", train_status->train_id ); 

			distance_to_reserve = 0; 
			int distance_since_deacceleration = 
				train_status->distance_since_speed_change - train_status->distance_before_deacceleration;

			distance_to_reserve = 
				get_train_stopping_distance( train_status->motion_data.train_speed, server_data) - distance_since_deacceleration; 

			// The train doesn't care if it gets the reserved distance or not; it should but it's already stopping anyway
			reserve_distance( distance_to_reserve, train_status, server_data );

			// Add sensors to attribution list
			add_sensors_attrib_list( train_status, server_data ); 

			break;
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Sensor Updates
// -----------------------------------------------------------------------------------------------------------------------------------------------------
void adjust_pos_with_sensor_data( Train_status *train_status, Train_server_data *server_data ){
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: adjust_pos_with_sensor_data-> Updating position [ train_id: %d curr_error: %d ]", 
		train_status->train_id, train_status->motion_data.current_error );

	int current_error = train_status->motion_data.current_error; 

	if ( current_error > 0 ){
		update_train_position_landmark( current_error, train_status, server_data->tasks_tids ); 
	}
	else if ( current_error < 0 ) {
		current_error = get_abs_value( current_error ); 
		update_train_position_neg_landmark( current_error, train_status, server_data ); 
	}

	// Reset the current error. 
	train_status->motion_data.current_error = 0; 
}

track_node *get_sensor_from_attr_list( Train_server_data *server_data, int sensor_index ){
	int i; 
	const char *ptr; 
	track_node *result = 0; 
	char *sensor_name = server_data->sensor_names[ sensor_index ];

	for ( i = 0; i < server_data->num_sensors_attr_list; i++ ){
		ptr = server_data->sensor_attr_list[i]->name; 
		if ( strcmp( sensor_name, ptr ) == 0 ){
			result = server_data->sensor_attr_list[i];
			break;
		}
	}

	return result; 
}

void add_sensors_attrib_list( Train_status *train_status, Train_server_data *server_data ){

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: add_sensors_attrib_list-> Updating sensor attribution list [ train_id: %d ]", 
		train_status->train_id);

	// Variables
	int distance_to_check = train_status->train_reservation.distance_reserved; 
	Train_reservation *curr_reservation = &train_status->train_reservation; 
	track_node *curr_node = curr_reservation->reservation_start.landmark; 
	track_edge *curr_edge = curr_reservation->reservation_start.edge; 
	
	// Remove the distance that we have already traveled.
	if ( curr_reservation->reservation_start.offset > 0 ){
		distance_to_check -= ( curr_edge->dist - curr_reservation->reservation_start.offset ); 
		curr_node = curr_edge->dest; 
	}

	// Is there a sensor in our route within the reserved distance? 
	int num_sensors_attr_list = 0; 
	while( distance_to_check > 0 ){
		if ( curr_node->type == NODE_SENSOR ){
			// This node is a sensor. Add it to the attribution list. 
			server_data->sensor_attr_list[num_sensors_attr_list] = curr_node; 
			num_sensors_attr_list++;
		}

		if ( curr_node->type == NODE_EXIT ){
			// There are no more nodes after this one. 
			break; 
		}
		else if ( curr_node->type == NODE_BRANCH ){
			// This node is a merge. Determine where the reserved distance goes to. 
			char curr_sw_pos = get_current_sw_pos( curr_node, train_status, server_data ); 

			if ( curr_sw_pos == SWITCH_STRAIGHT_POS ) {
				curr_edge = &curr_node->edge[ DIR_STRAIGHT ];
				curr_node = curr_edge->dest; 
			}
			else {
				curr_edge = &curr_node->edge[ DIR_CURVED ];
				curr_node = curr_edge->dest; 
			}
		}
		else{
			curr_edge = &curr_node->edge[ DIR_AHEAD ];
			curr_node = curr_edge->dest; 
		}

		distance_to_check -= curr_edge->dist; 
	}

	// Store the values in the attribution list
	server_data->num_sensors_attr_list = num_sensors_attr_list; 

	/*
	bwprintf( COM2, "SENSOR ATTR LIST UPDATED [ tr: %s offset: %d ]\n", 
		train_status->current_position.landmark->name, train_status->current_position.offset ); 
	int z; 
	for ( z = 0; z < num_sensors_attr_list; z++ ){
		bwprintf( COM2, " %s ", server_data->sensor_attr_list[ z ]->name ); 
	}
	bwprintf( COM2, "\n" ); 
	*/
}

track_node *get_sensor_triggered( Train_server_data *server_data ){
	// Initialization
	int i, sensor_found; 
	track_node *sensor_triggered = 0; 
	int *ptr_sensors_not = server_data->notifier_sensor_values;
	int *ptr_sensors = server_data->sensor_values;

	sensor_found = 0; 
	for( i = 0; i < NUM_SENSORS; i++ ){
		if ( *ptr_sensors_not == 1 && *ptr_sensors_not != *ptr_sensors && !sensor_found ){
			// If a sensor was triggered, and another sensor was previously found, try to see
			// if that sensor is part of the attribution list. 
			sensor_triggered = get_sensor_from_attr_list( server_data, i ); 
			if ( sensor_triggered )
				sensor_found = 1;
		}

		*ptr_sensors++ = *ptr_sensors_not++; // Copy the value
	}

	return sensor_triggered; 
}

int get_dist_between_sensor_and_landmark( track_node *triggered_sensor, Train_status *train_status, Train_server_data *server_data ){
	// Variables
	Train_reservation *curr_reservation = &train_status->train_reservation; 
	
	/*
	bwprintf( COM2, "CURRENT_POS: %s Offset: %d Triggered Sensor: %s \n", 
		train_status->current_position.landmark->name, train_status->current_position.offset, triggered_sensor->name ); 
	bwprintf( COM2, "CURRENT_RESERVATION: %s Offset: %d Distance: %d \n", 
		curr_reservation->reservation_start.landmark->name, curr_reservation->reservation_start.offset, curr_reservation->distance_reserved ); 
	bwprintf( COM2, "1 " ); 
	*/
	int sensor_found = 0; 
	int curr_landmark_found = 0; 
	int dist_between_landmarks = 0; 
	int distance_to_check = train_status->train_reservation.distance_reserved; 

	track_node *curr_node = curr_reservation->reservation_start.landmark; 
	track_edge *curr_edge = curr_reservation->reservation_start.edge; 
	//bwprintf( COM2, "2 " );

	if ( triggered_sensor != train_status->current_position.landmark ){
		//bwprintf( COM2, "3 " );
		// Remove the distance that we have already traveled.
		if ( curr_reservation->reservation_start.offset > 0 ){
			//bwprintf( COM2, "4 " );
			distance_to_check -= ( curr_edge->dist - curr_reservation->reservation_start.offset );
			curr_node = curr_edge->dest; 
		}

		//bwprintf( COM2, "5 " );
		while( distance_to_check > 0 && ( !sensor_found || !curr_landmark_found ) ){
			//bwprintf( COM2, "6 " );
			// Have we found any of the landmarks? 
			if ( curr_node == triggered_sensor )
				sensor_found = 1; 
			else if ( curr_node == train_status->current_position.landmark )
				curr_landmark_found = 1; 

			if ( sensor_found && curr_landmark_found )
				break; 

			// Determine where to move based on the landmark
			if ( curr_node->type == NODE_EXIT ){
				//bwprintf( COM2, "7 " );
				// There are no more nodes after this one. 
				break; 
			}
			else if ( curr_node->type == NODE_BRANCH ){
				//bwprintf( COM2, "8 " );
				// This node is a merge. Determine where the reserved distance goes to. 
				char curr_sw_pos = get_current_sw_pos( curr_node->reverse, train_status, server_data ); 
				//bwprintf( COM2, "9 " );

				if ( curr_sw_pos == SWITCH_STRAIGHT_POS ) {
					//bwprintf( COM2, "10 " );
					curr_edge = &curr_node->edge[ DIR_STRAIGHT ];
					curr_node = curr_edge->dest; 
				}
				else{
					//bwprintf( COM2, "11 " );
					curr_edge = &curr_node->edge[ DIR_CURVED ];
					curr_node = curr_edge->dest; 
				}
			}
			else{
				//printf( COM2, "12 " );
				curr_edge = &curr_node->edge[ DIR_AHEAD ];
				curr_node = curr_edge->dest; 
			}
			distance_to_check -= curr_edge->dist; 
			//bwprintf( COM2, "13 " );
			
			// Shall we add any distance? 
			if ( sensor_found )
				dist_between_landmarks -= curr_edge->dist;
			else if ( curr_landmark_found )
				dist_between_landmarks += curr_edge->dist; 
			//bwprintf( COM2, "14 " );
		}
		//bwprintf( COM2, "15 " );
		bwassert( sensor_found && curr_landmark_found, 
			"TRAIN_MOTION: get_dist_between_sensor_and_landmark: Both landmarks must be found" ); 
	}
	else{
		//bwprintf( COM2, "16 " );
		// Case : The triggered sensor is the same as the current position
		// - No need to check any other landmarks
	}

	// Substract the offset of the current position. 
	//bwprintf( COM2, "17 " );
	dist_between_landmarks -= train_status->current_position.offset; 
	//bwprintf( COM2, "18 [ Dist: %d ]\n", dist_between_landmarks );

	return dist_between_landmarks;
}

int update_with_sensor_data( track_node *triggered_sensor, Train_status *train_status, Train_server_data *server_data ){
	// Initialization
	bwdebug( DBG_USR, TEMP3_DEBUG_AREA, "TRAIN_MOTION: update_with_sensor_data -> UPDATE STARTS" ); 
	int error_distance = get_dist_between_sensor_and_landmark( triggered_sensor, train_status, server_data );

	// While moving, the current position of the train is not updated. The error distance is calculated and
	// as soon as the train stops its position is adjusted using the calculared error distance. 

	// Make the sensor the current position
	if ( train_status->motion_state == TRAIN_STILL ){
		// The train is not moving. We can safely adjust its position. 
		train_status->current_position.offset = 0; 
		train_status->current_position.landmark = triggered_sensor;
		train_status->current_position.edge = &triggered_sensor->edge[ DIR_AHEAD ];
	}
	else{
		// Update the error
		train_status->motion_data.current_error = error_distance;
	}

	//bwprintf( COM2, "Error Distance: %d Landmark: %s Current_Offset: %d\n", 
	//	error_distance, train_status->current_position.landmark->name, train_status->current_position.offset ); 

	train_status->motion_data.total_error += get_abs_value( error_distance ); 
	train_status->motion_data.num_error_measurements++; 

	return 1; 
}

