#include <userspace.h>

// -------------------------------------------------------------------
// Helper methods
// NOTE: Some of this methods might need to be in a different file. 
// -------------------------------------------------------------------
int position_equals( Train_position *position1, Train_position *position2 ){
	if ( position1 != position2 ){
		if ( position1 == 0 || position2 == 0 || 
			position1->landmark != position2->landmark || position1->offset != position2->offset )
			return 0; 
	}

	return 1; 
}

int has_train_arrived( Train_status *train_status ){
	if ( train_status->goal_state == FIXED_GOAL ){
		Train_position current_position = train_status->current_position; 
		Train_position goal_position = train_status->current_goal; 
		if ( current_position.landmark == goal_position.landmark && 
			current_position.offset >= goal_position.offset )
			return 1; 
	}
	return 0; 
}

void initialize_goal( Train_status *train_status ){
	// Erase the goal state
	train_status->goal_state = NO_GOAL; 

	// Reinitialize the goal values
	train_status->current_goal.edge = 0; 
	train_status->current_goal.landmark = 0; 
	train_status->current_goal.offset = 0; 
}

// -------------------------------------------------------------------
// Train Notifiers
// -------------------------------------------------------------------
/*
  There will be two notifiers
*/
void train_wait_notifier(){

}

void train_sensor_notifier(){

}

// -------------------------------------------------------------------
// Train Server
// -------------------------------------------------------------------

void update_train_status( Train_status *train_status, Calibration_data *calibration_data, int *servers_list ){

	if ( train_status == TRAIN_STILL ){
		// The train is still and the speed hasn't changed, so nothing to do here.
		return; 
	}

	// Initialization
	int current_time = TimeInMs();

	// Get the distance that the train has moved since the last update. 
	int distance_traveled = get_distance_traveled( current_time, train_status, calibration_data ); 

	// Get the distance from the last landmark. 
	update_train_position( distance_traveled, train_status, servers_list );
}

// TODO: Refactor the signature of these methods to make them cleaner. 
void update_train_goals( Train_update_request *update_request, Train_status *train_status, 
						Calibration_data *calibration_data, int *servers_list, track_node *track ){

	// Initialization
	Train_position *new_goal = update_request->new_goal;

	int cmd_server_tid = servers_list[TH_CMD_SERVER_TID_INDEX];
	int current_time = 0;
	int original_train_direction = train_status->direction; 

	// First update the current status of the train
	update_train_status( train_status, calibration_data, servers_list ); 

	// Check if there are new global goals to fulfill (they will erase previous goals). 
	// If there are not, continue working with the current one. 
	if ( update_request->update_type == GOAL_UPDATE_TYPE ){
		if ( new_goal == 0 ){
			// The train had a goal, but now it has been ordered to move freely. 
			// Remove the previous goal information.
			initialize_goal( train_status ); 

			// Send the commands
			int cmd_type = update_request->command.cmd_type;
			switch( cmd_type ){
				case TRAIN_CMD_TYPE: // This is a normal train command. Only update the speed and execute the command
					send_command( TRAIN_CMD_TYPE, train_status->train_id, update_request->command.param, cmd_server_tid );
					current_time = TimeInMs(); 
					train_status->current_speed.train_speed = update_request->command.param;
					train_status->time_speed_change = current_time;
					break; 
				case REVERSE_CMD_TYPE:
					// This is a reverse train command. A direction command is required
					// TODO: Is this going to track correctly the speed after the reverse? 
					
					// Execute reverse
					if ( train_status->direction == TRAIN_DIRECTION_FORWARD )
						train_status->direction = TRAIN_DIRECTION_REVERSE; 
					else
						train_status->direction = TRAIN_DIRECTION_FORWARD; 

					// Start moving again					
					send_command( REVERSE_CMD_TYPE, train_status->train_id, CMD_PARAM_NOT_REQUIRED, cmd_server_tid );
					current_time = TimeInMs(); 
					train_status->time_speed_change = current_time; 
					break;
				default:
					// Should never get here
					bwassert( 0, "TRAIN_SERVER: Invalid command type." ); 
					break;
			}
		} 
		else {
			// There's a new goal.
			// 1. Erase previous goal information. 
			initialize_goal( train_status ); 

			// 2. Get a new path. 
			request_new_path( train_status, servers_list, track );

			// 3. Update the detective. 
			// TODO: Update the detective to only track the sensors in the new route. 
		}
	}

	if ( train_status->goal_state == FIXED_GOAL ) {
		int stop_train = 0; 
		switch( train_status->motion_state ){
			case TRAIN_STILL:
				// Does the train have to reverse? 
				if ( train_status->is_reversing ){
					// Calculate the speed necessary for this part of the track
					Train_speed train_speed = calculate_speed_to_use( train_status, calibration_data );
					train_status->current_speed = train_speed; 

					// Execute reverse
					if ( train_status->direction == TRAIN_DIRECTION_FORWARD )
						train_status->direction = TRAIN_DIRECTION_REVERSE; 
					else
						train_status->direction = TRAIN_DIRECTION_FORWARD; 

					// Start moving again					
					// TODO: Make the swith change here. 
					send_command( REVERSE_CMD_TYPE, train_status->train_id, CMD_PARAM_NOT_REQUIRED, cmd_server_tid );
					send_command( TRAIN_CMD_TYPE, train_status->train_id, train_status->current_speed.train_speed, cmd_server_tid );
					current_time = TimeInMs(); 
					train_status->time_speed_change = current_time; 

					// Reverse is completed
					train_status->is_reversing = 0;
				}
				else if( has_train_arrived( train_status ) ) {
					// The train has reached its destination
					// Remove the current goal
					initialize_goal( train_status ); 
					// TODO: Print a message?
				}
				else{
					// The train hasn't started moving
					// Is the train facing the rigth direction? 
					if ( original_train_direction != train_status->direction ){
						// The train needs to move in the opposite direction; execute reverse.
						reverse_current_position( train_status );
						send_command( REVERSE_CMD_TYPE, train_status->train_id, CMD_PARAM_NOT_REQUIRED, cmd_server_tid );
					}

					// Calculate the speed necessary for this part of the track
					Train_speed train_speed = calculate_speed_to_use( train_status, calibration_data );
					train_status->current_speed = train_speed; 

					// Start moving
					send_command( TRAIN_CMD_TYPE, train_status->train_id, train_status->current_speed.train_speed, cmd_server_tid );
					current_time = TimeInMs(); 
					train_status->time_speed_change = current_time; 
				}
				break; 
			case TRAIN_ACCELERATING:
			case TRAIN_CONSTANT_SPEED:
			case TRAIN_DEACCELERATING:
				// Should the train start deaccelerating?
				stop_train = 0; 
				if ( train_status->current_speed.speed_type == SHORT_DISTANCE_SPEED ){
					// Short distance speed -> Time is used to determine if we should start de-accelerating
					current_time = TimeInMs(); 
					if ( current_time - train_status->time_speed_change <= train_status->current_speed.time_accelerating )
						stop_train = 1; 
				}
				else{	
					// Long distance speed -> Distance is used to determine if we should start de-accelerating
					int distance_to_travel = train_status->current_speed.distance_to_travel; 
					int stopping_distance = calibration_data->speed_data[ train_status->current_speed.train_speed - 1 ].stopping_distance; 

					// TODO: Should we add a buffer here, so that it starts de-accelerating before? 
					if ( distance_to_travel <= stopping_distance )
						stop_train = 1; 
				}

				if ( stop_train ){
					// Is the train stopping because it will need to reverse? 
					if ( train_status->current_speed.requires_reverse )
						train_status->is_reversing = 1;

					// Send the command to start de-accelerating
					send_command( TRAIN_CMD_TYPE, train_status->train_id, 0, cmd_server_tid );
					current_time = TimeInMs();
					train_status->time_speed_change = current_time;
				}

				break; 
		}
	}
}

void print_train_info(){
}

void initialize_servers_tids( int *tid_list ){
	int cmd_server_tid = WhoIs( COMMAND_SERVER_NAME ); 
	tid_list[ TH_CMD_SERVER_TID_INDEX ] = cmd_server_tid; 
	bwassert( cmd_server_tid >= 0, "TRAIN_SERVER: This task requires the command server to work properly." );

	int sw_server_tid = WhoIs( SWITCHES_SERVER_NAME );
	tid_list[ TH_SWITCH_SERVER_TID_INDEX ] = sw_server_tid; 
	bwassert( cmd_server_tid >= 0, "TRAIN_SERVER: This task requires the command server to work properly." );

	// TODO: Add the route server
	// TH_ROUTE_SERVER_TID_INDEX
}

void train_server(){
	// Initialization
	int sender_tid, update_status; 
	char temp_buffer[ COURIER_BUFFER_SIZE ];
	Train_position *current_goal; 

	track_edge edges[MAX_NUM_EDGES];
	track_node landmarks[MAX_NUM_LANDMARKS]; 
	char switches_state[NUM_SWITCHES]; 

	Train_status train_status; 
	train_status.route_data.edges = edges; 
	train_status.route_data.landmarks = landmarks;
	train_status.route_data.switches_state = switches_state; 

	Train_update_request train_update_request;

	// Get track data
	track_node *track;
	Train_initialization_msg train_initialization; 
	Receive( &sender_tid, ( char * ) &train_initialization, sizeof( train_initialization ) );
	Reply( sender_tid, 0, 0 ); 

	train_status.train_id = train_initialization.train_id; 
	train_status.direction = train_initialization.direction; 
	track = train_initialization.track; 

	// Get Calibration data
	Calibration_data calibration_data;
	load_calibration_data( &calibration_data, train_status.train_id );

	// Create notifiers
	int wait_notifier_tid = Create( TRAIN_WAIT_NOT_TASK_PRIORITY, train_wait_notifier ); 
	int sensor_notifier_tid = Create( TRAIN_SENSOR_NOT_TASK_PRIORITY, train_sensor_notifier ); 

	// Couriers
	Courier_msg courier_msg; 
	int wait_notifier_courier = create_courier( MyTid(), wait_notifier_tid, 0, 0 );
	int sensor_notifier_courier = create_courier( MyTid(), sensor_notifier_tid, 0, 0 );

	// Commands
	Cmd_request cmd_request;

	// Get the server tids. 
	int servers_tid[TRAIN_NUM_SERVERS_TIDS];
	initialize_servers_tids( servers_tid );

	FOREVER{
		// Receive message
		bwdebug( DBG_USR, SWITCHES_SERVER_DEBUG_AREA, "TRAIN_SERVER: listening for a request" );
		Receive( &sender_tid, temp_buffer, sizeof( char ) * COURIER_BUFFER_SIZE );
		
		update_status = 0; 
		if ( sender_tid == wait_notifier_courier || sender_tid == sensor_notifier_courier ){
			// Server received a courier message
			mem_cpy( temp_buffer, ( char * ) &courier_msg, sizeof( courier_msg ) );

			if ( sender_tid == sensor_notifier_courier ){
				// Update state from sensor data. 
				bwdebug( DBG_USR, SWITCHES_SERVER_DEBUG_AREA, 
					"TRAIN_SERVER: Received sensor courier message [ sender_tid: %d ]", sender_tid );
				// TODO: Update state from sensor data.
			}
			else{
				bwdebug( DBG_USR, SWITCHES_SERVER_DEBUG_AREA, 
					"TRAIN_SERVER: Received wait courier message [ sender_tid: %d ]", sender_tid );
			}
			
			train_update_request.update_type = NORMAL_UPDATE_TYPE; 
			update_status = 1;
		}
		else{
			// Server received a command message
			mem_cpy( temp_buffer, ( char * ) &cmd_request, sizeof( cmd_request ) ); 
			bwdebug( DBG_USR, SWITCHES_SERVER_DEBUG_AREA, 
				"TRAIN_SERVER: Received command [ sender_tid: %d cmd_type: %d ]", sender_tid, cmd_request.cmd.cmd_type );

			if ( cmd_request.cmd.cmd_type == TRAIN_CMD_TYPE || cmd_request.cmd.cmd_type == REVERSE_CMD_TYPE ){
				// A command was received to make the train move freely
				train_update_request.update_type = GOAL_UPDATE_TYPE; 
				train_update_request.command = cmd_request.cmd;
				update_status = 1; 
			}
			else if ( cmd_request.cmd.cmd_type == MOVE_TO_POSITION_CMD_TYPE ){
				// A command was made to make the train move to a particular location
				train_update_request.update_type = GOAL_UPDATE_TYPE;
				train_update_request.new_goal = ( Train_position * ) cmd_request.cmd.param; // TODO: Be careful with this
				update_status = 1; 
			}
		}

		// Update status
		if ( update_status ){
			update_train_goals( &train_update_request, &train_status, &calibration_data, servers_tid, track );
		}
		else{
			bwassert( 0, "TRAIN_SERVER: The received messages must be valid." );
		}
	}
}
