#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Command Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------------
void send_train_move_command( int tr_speed, Train_status *train_status, Train_server_data *server_data ){
	// First, get the command server tid
	int cmd_server_tid = server_data->tasks_tids[TR_CMD_SERVER_TID_INDEX]; 

	// If it's the same speed ignore this request. 
	if ( tr_speed == train_status->motion_data.train_speed )
		return;

	// Is velocity enabled?
	if ( tr_speed != TRAIN_STOP_CMD_SPEED && !server_data->calibration_data.speed_data[ tr_speed - 1 ].velocity_enabled )
		bwassert( 0, "TRAIN_MOTION: Velocity not enabled." ); 

	// Send the command to the server
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Sending TR command. [ train_id: %d speed: %d]", 
		train_status->train_id, tr_speed );
	send_command( TRAIN_CMD_TYPE, train_status->train_id, tr_speed, cmd_server_tid );

	int current_time = TimeInMs(); 
	 
	train_status->time_speed_change = current_time;
	
	if ( tr_speed == TRAIN_STOP_CMD_SPEED ){
		train_status->motion_state = TRAIN_DEACCELERATING; 
		train_status->distance_before_deacceleration = train_status->distance_since_speed_change; 
	}
	else {
		train_status->motion_state = TRAIN_ACCELERATING; 
		train_status->motion_data.train_speed = tr_speed;
		train_status->distance_since_speed_change = 0;
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
  This method returns the distance traveled since the last refresh. 
*/
int get_distance_traveled( int current_time, Train_status *train_status, Train_server_data *server_data ){

	// NOTE:
	// Because of the nature of our project we don't change between speeds -> The only 
	// change from speed 0 to another speed, and then, at some point, we go back to 0.
	int time_since_change = current_time - train_status->time_speed_change;
	if ( time_since_change <= 0 ){
		return 0; 
	}

	int distance_type = train_status->motion_data.distance_type;
	int distance_traveled = 0; 

	if ( distance_type == SHORT_DISTANCE )
		distance_traveled = get_short_distance_traveled( time_since_change, train_status, &( server_data->calibration_data ) ); 
	else
		distance_traveled = get_long_distance_traveled( time_since_change, train_status, &( server_data->calibration_data ) ); 

	distance_traveled += train_status->motion_data.current_error; 

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
                distance_traveled += ( int )( time_constant_speed * speed_calibration->velocity ); 

				bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
					"TRAIN_SERVER: get_long_distance_traveled -> Constant Speed. [ time_since_ch: %d time_to_speed: %d  dist: %d ]", 
					time_since_change, speed_calibration->time_to_constant_speed,  distance_traveled ); 
        }
    }
    else if ( motion_state == TRAIN_DEACCELERATING ){
        // Since we don't know the (de) acceleration rate, but only the stopping distance and stopping time.
        // Even though it's not completely accurate, we just make a linear interpolation. 
                
        // Previous speed
        distance_traveled = train_status->distance_before_deacceleration;

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

    // distance_traveled contains the distance since the speed change. Now this distance is compared to the 
    // one calculated during the previous refresh. 
    // TODO: After checking the sensors remember to correctly update this parameters to avoid having negative values. 
    int distance_since_change = distance_traveled - train_status->distance_since_speed_change;
    bwassert( distance_since_change >= 0, "TRAIN_MOTION: get_long_distance_traveled -> The distance cannot be negative." ); 

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
		int distance_traveled, time, total_distance_in_speed, total_travel_time; 
		Train_motion_data *motion_data = &train_status->motion_data;
		Speed_calibration_data *speed_calibration = &(calibration_data->speed_data[motion_data->train_speed]); 

		total_distance_in_speed = get_short_distance( motion_data->train_speed, speed_calibration );
		total_travel_time = get_short_distance_total_time( motion_data->train_speed, speed_calibration ); 
		time = total_travel_time - time_since_change;

		if ( time > 0 ){
			// Train is still moving. 
			distance_traveled = 
				( total_distance_in_speed * time_since_change ) / ( total_travel_time );
		}
		else{
			// The train has already stopped
			distance_traveled = total_distance_in_speed; 
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
int get_straight_distance( Train_status *status, int *requires_reverse ){
        // Initialization
        int is_reverse = 0; 
        int total_straight_distance = 0; 
        int landmark_index = status->route_data.landmark_index; 
        int num_landmarks = status->route_data.num_landmarks; 

        bwassert( landmark_index < num_landmarks, 
                "TRAIN_MOTION: get_straight_distance -> The landmark index must be less than the num of landmarks." ); 

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

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Speed Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------------
/*
  This method determines which speed to use to reach a particular position. 
  Return: 
  - The best speed to use to reach a requested location. It also says if it is a "short" or
    "long" distance. 
*/
void calculate_speed_to_use( Train_status *status, Calibration_data *calibration_data ){
	// Initialization
	int speed_to_use, index, speed_found, distance_in_speed; 
	Speed_calibration_data *speed_data; 

    // Calculate the speed required to travel the total straight distance. 
    int requires_reverse = 0; 
    int total_straight_distance = get_straight_distance( status, &requires_reverse );
    if ( total_straight_distance <= 0 ){
		// The train doesn't have to move. 
		status->motion_data.distance_type = SHORT_DISTANCE;
		status->motion_data.train_speed = 0; 
    }

    // Set the distance that this speed will cover. 
	status->motion_data.distance_to_travel = total_straight_distance; 
	status->motion_data.requires_reverse = requires_reverse; 

    // Does the train require short speed? 
    if ( total_straight_distance <= calibration_data->short_speed_cutoff ){

        // Which short distance should we use? 
		status->motion_data.distance_type = SHORT_DISTANCE; 
        speed_to_use = MAX_SPEED; 
        speed_found = 0; 

        while( speed_to_use > 0 && !speed_found ){
			// Iterate from the highest speed to the lowest
            speed_data = &( calibration_data->speed_data[ --speed_to_use ] ); 
                        
            for ( index = 0; index < NUM_CALIBRATED_DISTANCES; index++ ){
                // This speed is too high; better use a lower one
				distance_in_speed = get_short_distance( index, speed_data ); 
                if ( index == 0 && distance_in_speed > total_straight_distance )
                     break;

                if ( distance_in_speed > total_straight_distance || distance_in_speed == 0  ){
                    // We found a close speed that travels a longer distance than what we expected; we need to use
                    // the previous one. 
                    speed_found = 1; 
                    break; 
                }
                else if ( distance_in_speed == total_straight_distance ){
                        // A perfect match was found. 
                        speed_found = 2; 
                }
            }

            if ( index == NUM_CALIBRATED_DISTANCES || speed_found ){
                if ( speed_found != 2 )
                        index--;

                // TODO: If the distance is not completely precise, should we calculate an interpolation? 
				status->motion_data.train_speed = speed_to_use + 1; 
            }
        }
    }
    else{
		status->motion_data.distance_type = LONG_DISTANCE;
        speed_to_use = MAX_SPEED; 
        speed_found = 0;

        while( speed_to_use > 0 && !speed_found ){
                // Iterate from the highest speed to the lowest
                speed_data = &( calibration_data->speed_data[ --speed_to_use ] ); 

                if ( speed_data->stopping_distance + speed_data->distance_during_acceleration < total_straight_distance )
                        speed_found = 1; 
        }

		status->motion_data.train_speed = speed_to_use + 1; 
		bwassert( status->motion_data.train_speed > 0, 
                "TRAIN_MOTION: calculate_speed_to_use -> The train must move; the speed cannot be less than 1." ); 
    }
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Landmark Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------------
int has_train_arrived( Train_status *train_status ){
	if ( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
        Train_position current_position = train_status->current_position; 
        Train_position goal_position = train_status->current_goal; 
        if ( current_position.landmark == goal_position.landmark && 
            current_position.offset >= goal_position.offset )
            return 1; 
    }
    return 0; 
}

void update_train_position_landmark( int distance_since_update, Train_status *train_status, int *servers_list ){
    bwassert( distance_since_update >= 0, 
            "TRAIN_MOTION: update_train_position -> The distance cannot be negative." ); 

    // First, calculate the distance after the previous landmark
    int distance_from_landmark = train_status->current_position.offset + distance_since_update; 

    // Second, calculate how far away is the train from the next landmark
    int distance_to_landmark = train_status->current_position.edge->dist; 

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
        "TRAIN_MOTION: update_train_position_landmark -> "
		"Calculating new landmark and offset. [ train_id: %d distance_from: %d distance_to: %d ]", 
        train_status->train_id, distance_from_landmark, distance_to_landmark );

    if ( ( distance_to_landmark - distance_from_landmark ) > 0 ){
        // The train hasn't reached a new landmark
        train_status->current_position.offset = distance_from_landmark; 
    }
    else{
        // The train has reached a new landmark
        train_status->current_position.landmark = train_status->current_position.edge->dest; 

        // Which type of landmark is it? 
		int landmark_type = train_status->current_position.landmark->type;
        if ( landmark_type == NODE_BRANCH ){
            // The new node is a switch. 
            char temp_str[6]; 
            int switch_id = atoi( substr( temp_str, train_status->current_position.landmark->name, 2, 6 ) );

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
                train_status->current_position.edge = &( train_status->current_position.landmark->edge[DIR_STRAIGHT] );
            }
            else if ( query_msg_reply.switch_position == SWITCH_CURVE_POS ){
                train_status->current_position.edge = &( train_status->current_position.landmark->edge[DIR_CURVED] );
            }
            else {
                bwassert( 0, "TRAIN_MOTION: update_train_position -> Invalid switch position [ sw_id: %d pos: %c ]", 
					query_msg_reply.switch_id, query_msg_reply.switch_position ); 
            }

        }
        else {
                train_status->current_position.edge = &( train_status->current_position.landmark->edge[DIR_AHEAD] );
        }

		if( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
            // There's a goal, so update it's landmark index. 
            train_status->route_data.landmark_index++;
            bwassert( train_status->route_data.landmark_index < train_status->route_data.num_landmarks, 
                    "TRAIN_MOTION: update_train_position -> The landmark index must be less than the num of landmarks." ); 
        }

        train_status->current_position.offset = distance_from_landmark - distance_to_landmark; 
    }
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Train Motion
// -----------------------------------------------------------------------------------------------------------------------------------------------------
void update_train_status( Train_update_request *update_request, Train_status *train_status, Train_server_data *server_data ){
	
	// Initialization
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: Starting train update. [ update_type: %d ]", update_request->update_type );
	int original_train_direction = train_status->train_direction; 

	// 1. Calculate the current position of the train. 
	update_train_position( train_status, server_data ); 

	// 2. Handle the update request
	handle_update_request( update_request, train_status, server_data ); 

	// 3. Determine if a command needs to be sent to the train. 
	int stop_train = 0; 
	switch( train_status->motion_state ){
		case TRAIN_STILL:
			if ( train_status->is_reversing ){
				// Send the reverse command
				send_reverse_command( train_status, server_data ); 

				// The reversing is completed. 
				train_status->is_reversing = 0; 
			}
			else if ( has_train_arrived( train_status ) ){
				// The train has reached its destination
                // Remove the current goal
				initialize_goal( train_status ); 

				// TODO: Remove the sensor tracking of this part of the track 
			}
			else{
				// The train hasn't started moving. 
				if ( original_train_direction != train_status->train_direction ){
					// The train needs to move in the opposite direction -> execute reverse.
					train_status->train_direction = original_train_direction;	// TODO: Check if Pavel is chaning the direction. If not remove this hack. 
					send_reverse_command( train_status, server_data ); 
				}
				else{
					int speed_to_use = 0; 
					if ( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
						calculate_speed_to_use( train_status, &server_data->calibration_data ); 
						speed_to_use = train_status->motion_data.train_speed; 
						send_train_move_command( speed_to_use, train_status, server_data );  
					}
					else if ( train_status->train_state == TRAIN_STATE_MOVE_FREE && train_status->motion_data.original_train_speed != 0 ){
						speed_to_use = train_status->motion_data.original_train_speed; 
						send_train_move_command( speed_to_use, train_status, server_data );  
					}
				}
			}
			break; 
        case TRAIN_ACCELERATING:
		case TRAIN_CONSTANT_SPEED:
		case TRAIN_DEACCELERATING:
			if ( train_status->train_state == TRAIN_STATE_MOVE_TO_GOAL ){
				// Should the train start de-accelerating?
				stop_train = 0;
				int current_time; 
				if ( train_status->motion_data.distance_type == SHORT_DISTANCE ){
					// Short distance speed -> Time is used to determine if we should start de-accelerating
					current_time = TimeInMs(); 
					if ( current_time - train_status->time_speed_change <= train_status->motion_data.time_accelerating )
						stop_train = 1; 
				}
				else{   
                    // Long distance speed -> Distance is used to determine if we should start de-accelerating
					int distance_to_travel = train_status->motion_data.distance_to_travel; 
					int stopping_distance = server_data->calibration_data.speed_data[ train_status->motion_data.train_speed - 1 ].stopping_distance; 

                    // TODO: Should we add a buffer here, so that it starts de-accelerating before? 
                    if ( distance_to_travel <= stopping_distance )
                            stop_train = 1; 
                 }

                if ( stop_train ){
                    // Is the train stopping because it will need to reverse? 
					if ( train_status->motion_data.requires_reverse )
                        train_status->is_reversing = 1;

                    // Send the command to start de-accelerating
					send_train_move_command( TRAIN_STOP_CMD_SPEED, train_status, server_data );
                }
			}
			break; 
	}

	// 4. Print the information. 
	print_train_status( train_status ); 
}

void update_train_position( Train_status *train_status, Train_server_data *server_data ){

	if ( train_status->motion_state == TRAIN_STILL ){
		// The train is still and the speed hasn't changed, so nothing to do here.
		bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
			"TRAIN_MOTION: update_train_status -> Train is not moving; there's nothing to update. [ train_id: %d ]", train_status->train_id );
		return; 
	}

	// Initialization
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_MOTION: update_train_status -> Starting train status update. [ train_id: %d ]", train_status->train_id );
	int current_time = TimeInMs();

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
				send_train_move_command( update_request->cmd.param, train_status, server_data ); 
				break; 
			case REVERSE_CMD_TYPE:
				train_status->motion_data.original_train_speed = train_status->motion_data.train_speed; 
				send_train_move_command( TRAIN_STOP_CMD_SPEED, train_status, server_data );
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

		// 2. Set the state of the train to GOAL
		train_status->train_state = TRAIN_STATE_MOVE_TO_GOAL; 

		// 3. Get a new path
		request_new_path( train_status, server_data );

        // 4. Update the detective. 
        // TODO: Update the detective to only track the sensors in the new route.
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Sensor Updates
// -----------------------------------------------------------------------------------------------------------------------------------------------------
int is_sensor_in_attrib_list( Train_server_data *server_data, int sensor_index ){
	int i; 
	const char *ptr; 
	char *sensor_name = server_data->sensor_names[ sensor_index ];

	for ( i = 0; i < server_data->num_sensors_attr_list; i++ ){
		ptr = server_data->sensor_attr_list[i]->name;
		if ( strcmp( sensor_name, ptr ) == 0 )
			return 1; 
	}

	return 0; 
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
			is_sensor_in_attrib_list( server_data, i );
		}

		*ptr_sensors = *ptr_sensors_not; // Copy the value
	}

	return sensor_triggered; 
}

int update_with_sensor_data( track_node *triggered_sensor, Train_status *train_status, 
							 Train_server_data *server_data ){
	// Initialization
	int error_distance; 
	track_node *prev_node; 
	Train_position current_pos = train_status->current_position;

	// What's the distance from the calculated position to the actual position? 
	// NOTE: From the current position we check the next and previous landmark and see if
	// the triggered sensor is there. If not, the train is declared lost. 
	if ( current_pos.landmark == triggered_sensor ){
		error_distance = -current_pos.offset;
	}
	else if( current_pos.edge->dest == triggered_sensor ){
		error_distance = current_pos.edge->dist - current_pos.offset;
	}
	else{ 
		int sensor_found = 0; 
		prev_node = get_prev_node( &current_pos );

		if ( prev_node ){
			if ( prev_node == triggered_sensor ){
				sensor_found = 1; 
				error_distance = current_pos.landmark->reverse->edge[ DIR_AHEAD ].dist;
			}
		}
		else if( current_pos.landmark->type == NODE_MERGE ) {
			// The current node is a merge
			track_node *straight_node = current_pos.landmark->reverse->edge[ DIR_STRAIGHT ].dest;
			track_node *reverse_node = current_pos.landmark->reverse->edge[ DIR_CURVED ].dest; 

			if ( straight_node == triggered_sensor ){
				sensor_found = 1; 
				error_distance = current_pos.landmark->reverse->edge[ DIR_STRAIGHT ].dist;
			}
			else if( reverse_node == triggered_sensor ){
				sensor_found = 1; 
				error_distance = current_pos.landmark->reverse->edge[ DIR_CURVED ].dist;
			}
		}

		if( sensor_found ){
			error_distance = ( error_distance * -1 ) - current_pos.offset; 
		}
		else{
			bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
				"TRAIN_MOTION: update_with_sensor_data -> The train is lost [ train_id: %d ]", 
				train_status->train_id );
			// TODO: Mark the train status as lost
			// Stop the train right away. 
			return 0; 
		}
	}

	// Make the sensor the current position
	// TODO: Do we need to calculate the time the train spent processing? 
	train_status->current_position.offset = 0; 
	train_status->current_position.landmark = triggered_sensor;
	train_status->current_position.edge = &triggered_sensor->edge[ DIR_AHEAD ];

	// Update the error
	train_status->motion_data.current_error += error_distance;
	return 1; 
}


