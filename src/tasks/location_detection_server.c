#include <userspace.h>

// -------------------------------------------------------------------
// Location Detection Notifier
// -------------------------------------------------------------------
void get_sensor_names( int sensor_server_tid, Location_notifier_data *notifier_data ){

	bwdebug( 
		DBG_USR, LOCATION_SRV_DEBUG_AREA, "LOCATION_DETECTION_NOT: Retrieving sensors list [ sensor_srv_tid: %d ]", 
		sensor_server_tid );

	// Initialization
	Sensor_msg sensor_msg;
	sensor_msg.type = GET_SENSOR_LIST_MSG; 

	Sensor_id_list_reply sensor_id_list_reply;

	// Get the list of sensors
	Send( sensor_server_tid, ( char * ) &sensor_msg, sizeof( sensor_msg ), ( char * ) &sensor_id_list_reply, 
		sizeof( sensor_id_list_reply ) );

	// Copy the values to this task's address space
	int i; 
	for( i = 0; i < NUM_SENSORS; i++ ){
		mem_cpy( sensor_id_list_reply.sensors_name[i], 
			notifier_data->sensor_names[i], SENSOR_NAME_SIZE );
	}

	bwdebug( DBG_USR, LOCATION_SRV_DEBUG_AREA, "LOCATION_DETECTION_NOT: Sensors retrieved succesfully [ sensor_srv_tid: %d ]", 
			sensor_server_tid );
}

track_node *get_sensor_node_from_index( int sensor_index, Location_notifier_data *notifier_data ){
	// Initialization
	int i; 
	track_node *result; 

	track_node *track = notifier_data->track; 
	char *sensor_name = notifier_data->sensor_names[ sensor_index ];

	for( i = 0; i < NUM_SENSORS; i++ ){
		if ( strcmp( sensor_name, track[ i ].name ) == 0 ){
			result = &track[ i ];
			break; 
		}
	}

	bwassert( result > 0, "LOCATION_DETECTION_NOT: The node of a sensor must always be found." ); 
	return result; 
}

int get_triggered_sensor_index( int *new_sensor_vals, Location_notifier_data *notifier_data ){
	// Initialization
	int i, sensor_index; 
	int *curr_sensor_vals = notifier_data->curr_sensor_vals; 

	sensor_index = 0; 
	for ( i = 0; i < NUM_SENSORS; i++ ){
		if ( *new_sensor_vals == 1 && *curr_sensor_vals != *new_sensor_vals && !sensor_index ){
			sensor_index = 1; 
		}

		*curr_sensor_vals++ = *new_sensor_vals++;
	}

	return sensor_index; 
}

int calculate_distance_from_sensor( Location_notifier_data *notifier_data ){
	return 0; 
}

void location_server_notifier(){
	bwdebug( DBG_SYS, LOCATION_SRV_DEBUG_AREA, "LOCATION_DETECTION_NOT: Enters" ); 

	// Variables
	int location_server_tid, init_time, end_time; 
	int default_speed = 1; 
	Location_notifier_data notifier_data; 

	// Tasks Tids
	int sensor_server_tid = WhoIs( SENSORS_SERVER_NAME ); 
	bwassert( sensor_server_tid > 0, "LOCATION_DETECTION_NOT: This taks requires of the sensor server to be able to work" ); 

	int cmd_server_tid = WhoIs( COMMAND_SERVER_NAME ); 
	bwassert( sensor_server_tid > 0, "LOCATION_DETECTION_NOT: This taks requires of the command server to be able to work" ); 

	// Get the list of sensors
	get_sensor_names( sensor_server_tid, &notifier_data ); 

	// Messages
	Sensor_msg sensor_msg; 
	Location_notifier_msg notifier_msg; 
	Location_server_msg location_srv_msg; 
	Sensor_update_reply sensor_update_reply; 

	FOREVER{
		// Wait for a request
		bwdebug( DBG_SYS, LOCATION_SRV_DEBUG_AREA, "LOCATION_DETECTION_NOT: Listening for a request" ); 
		Receive( &location_server_tid, ( char * ) &notifier_msg, sizeof( notifier_msg ) ); 
		Reply( location_server_tid, 0, 0 ); 

		// Store the required data
		notifier_data.track = notifier_msg.track; 

		// 1. Get all sensor current data
		sensor_msg.type = GET_SENSORS_CURR_VALUE; 
		sensor_msg.sensors_val_destination = notifier_data.curr_sensor_vals; 
		Send( sensor_server_tid, ( char * ) &sensor_msg, sizeof( sensor_msg ), 0, 0 ); 

		// 2. Start train moving and measure current time
		init_time = TimeInMs(); 
		send_command( TRAIN_CMD_TYPE, notifier_msg.train_to_find, default_speed, cmd_server_tid );

		// 3. Wait for sensors. 
		//	The moment any sensor that was not triggered before changes, then it's the sensor 
		//	we are waiting for. 
		int triggered_sensor_index = 0; 
		sensor_msg.type = WAIT_ALL_SENSORS_CHANGE_MSG; 
		while( !triggered_sensor_index ){
			Send( sensor_server_tid, ( char * ) &sensor_msg, sizeof( sensor_msg ), 
				( char * ) &sensor_update_reply , sizeof( sensor_update_reply ) ); 

			// Check to see which sensor, if any, was triggered. 
			triggered_sensor_index = get_triggered_sensor_index( sensor_update_reply.sensors_value, &notifier_data );
		}

		// 4. Stop the train right away. 
		send_command( TRAIN_CMD_TYPE, notifier_msg.train_to_find, default_speed, cmd_server_tid );

		// 5. Get the track node this sensor belongs to. 
		track_node *sensor_triggered = get_sensor_node_from_index( triggered_sensor_index, &notifier_data ); 

		// 6. Calculate the stopping distance. 
		//	Based on speed calibration
		//	If it was below the constant speed use the table otherwise it's the stopping distance.
		//  NOTE: This also delays the required time required for the train to stop completely.
		int distance_from_sensor = calculate_distance_from_sensor( &notifier_data ); 

		// 7. Inform the location sever that the train was found. 
		// TODO: Do we need to consider the distance from the sensor? 
		location_srv_msg.type = LOC_SRV_TRAIN_FOUND; 
		location_srv_msg.train_location = sensor_triggered; 
		location_srv_msg.args = distance_from_sensor; 
		Send( location_server_tid, ( char * ) &location_srv_msg, sizeof( location_srv_msg ), 0, 0 ); 
	}
}

// -------------------------------------------------------------------
// Location Detection Server
// -------------------------------------------------------------------
void init_loc_srv_state( Location_server_data *server_data ){
	// Initialize variables
	server_data->train_to_find = 0; 

	// Initialize flags
	server_data->has_find_train_request = 0;
	server_data->are_trains_stopped = 0; 
	server_data->is_usr_ready = 0; 

	// Initialize train data
	int i; 
	for ( i = 0; i < NUM_TRAINS; i++ ){
		server_data->trains_data[i].train_tid = 0; 
		server_data->trains_data[i].train_num = 0; 
		server_data->trains_data[i].train_status = LC_TRAIN_STATUS_UNKNOWN;
	}
}

void add_find_train_request_info( Location_server_msg *msg, Location_server_data *server_data ){
	if ( !server_data->has_find_train_request ){
		server_data->train_to_find = msg->param; 
		server_data->track = msg->track; 
		
		int i; 
		for( i = 0; i < NUM_TRAINS; i++ ){
			if ( !msg->train_nums[i] )
				break; 
			server_data->trains_data[i].train_num = msg->train_nums[i];
			server_data->trains_data[i].train_tid = msg->train_tids[i]; 
			server_data->trains_data[i].train_status = LC_TRAIN_STATUS_UNKNOWN;
		}

		server_data->has_find_train_request = 1; 
	}
	else{
		bwdebug( 
			DBG_SYS, LOCATION_SRV_DEBUG_AREA, 
			"LOCATION_DETECTION_SRV: Server is already working on another request. The new one will be ignored." ); 
	}
}

void add_train_stopped_info( int train_id, Location_server_data *server_data ){
	int i; 
	int num_trains_stopped = 0; 
	int found = 0; 

	for( i = 0; i < NUM_TRAINS; i++ ){
		if ( server_data->trains_data[ i ].train_num == train_id ){
			server_data->trains_data[ i ].train_status = LC_TRAIN_STATUS_STOPPED;
			found = 1;
		}

		if ( server_data->trains_data[ i ].train_status == LC_TRAIN_STATUS_STOPPED )
			num_trains_stopped++; 
	}

	bwassert( 
		found, "LOCATION_DETECTION_SRV: The train must always be found [ train_id: %d ]", train_id ); 
}

int get_train_index( int train_num, Location_server_data *server_data ){
	int i;
	int train_index = -1; 

	for ( i = 0; i < NUM_TRAINS; i++ ){
		if ( server_data->trains_data[i].train_num == train_num ){
			train_index = i;
			break; 
		}
	}

	return train_index; 
}

void send_train_new_location( track_node* new_landmark, int new_offset, Location_server_data *server_data ){
	int train_index = get_train_index( server_data->train_to_find, server_data ); 
	if ( train_index >= 0 && train_index < NUM_TRAINS ){
		Cmd_request cmd_request;
		cmd_request.type = ADD_CMD_REQUEST; 
		cmd_request.cmd.cmd_type = UPDATE_TRAIN_POS_CMD_TYPE; 
		cmd_request.cmd.element_id = ( int ) new_landmark; 
		cmd_request.cmd.param = new_offset; 

		int train_tid = server_data->trains_data[ train_index ].train_tid; 
		Send( train_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0 ); 
	}
	else
		bwassert( 0, "LOCATION_DETECTION_SRV: The train must always be found" ); 
}

void reenable_trains( Location_server_data *server_data ){
	// Inform each train that it's ready to move.
	Cmd_request cmd_request;
	cmd_request.type = ADD_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = RESTART_TRAIN_CMD_TYPE; 
	cmd_request.cmd.element_id = 0;
	cmd_request.cmd.param = 0;

	int i, train_tid; 
	for ( i = 0; i < NUM_TRAINS; i++ ){
		train_tid = server_data->trains_data[ i ].train_tid; 
		if ( train_tid > 0 )
			Send( train_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0 );
	}
}

void stop_trains( Location_server_data *server_data ){
	// Inform each train that it's ready to move.
	Cmd_request cmd_request;
	cmd_request.type = ADD_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = STOP_TRAIN_CMD_TYPE; 
	cmd_request.cmd.element_id = 0;
	cmd_request.cmd.param = 0;

	int i, train_tid; 
	for ( i = 0; i < NUM_TRAINS; i++ ){
		train_tid = server_data->trains_data[ i ].train_tid; 
		if ( train_tid > 0 )
			Send( train_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0 );
	}
}

void location_detection_server(){
	// Initialization
	bwdebug( DBG_SYS, LOCATION_SRV_DEBUG_AREA, "LOCATION_DETECTION_SRV: Enters" ); 
	RegisterAs( LOCATION_SERVER_NAME ); 

	// Create the notifier. 
	int notifier_tid = Create( TRAIN_LOCATION_NOT_PRIORITY, location_server_notifier ); 
	bwdebug( DBG_SYS, LOCATION_SRV_DEBUG_AREA, 
		"LOCATION_DETECTION_SRV: Created location notifier successfully [ not_tid: %d ]", notifier_tid ); 
	
	// Variables
	int sender_tid; 
	Location_server_data server_data; 
	init_loc_srv_state( &server_data ); 

	// Messages
	Location_server_msg location_srv_msg; 
	Location_notifier_msg notifier_msg; 

	FOREVER{
		// Get the request
		bwdebug( DBG_SYS, LOCATION_SRV_DEBUG_AREA, "LOCATION_DETECTION_SRV: Listening for a request." ); 
		Receive( &sender_tid, ( char * ) &location_srv_msg, sizeof( location_srv_msg ) ); 

		switch( location_srv_msg.type ){
			case LOC_SRV_FIND_TRAIN_REQ:		// Request 1: Find train. 
				bwdebug( 
					DBG_SYS, LOCATION_SRV_DEBUG_AREA, 
					"LOCATION_DETECTION_SRV: Received find train request. [ sender_tid: %d train_to_find: %d ]",
					sender_tid, location_srv_msg.param );
				add_find_train_request_info( &location_srv_msg, &server_data ); 
				stop_trains( &server_data ); 

				break;
			case LOC_SRV_TRAIN_STOPPED:			// Request 2: Train stopped.
				bwdebug( 
					DBG_SYS, LOCATION_SRV_DEBUG_AREA, 
					"LOCATION_DETECTION_SRV: Received train stopped notification. [ train_id: %d train_tid: %d ]",
					location_srv_msg.param, sender_tid );
				
				// Store the information about the train that has just been stopped. 
				add_train_stopped_info( location_srv_msg.param, &server_data ); 

				break;
			case LOC_SRV_USR_READY:				// Request 3: User ready
				// This is not used for now.  
				break; 
			case LOC_SRV_TRAIN_FOUND:			// Request 4: Train found. 

				bwdebug( DBG_SYS, LOCATION_SRV_DEBUG_AREA, "LOCATION_DETECTION_SRV: Train found!." ); 
				send_train_new_location( location_srv_msg.train_location, location_srv_msg.args, &server_data ); 
				reenable_trains( &server_data );

				// Reset all information
				init_loc_srv_state( &server_data );

				break; 
		}

		if ( server_data.has_find_train_request && server_data.are_trains_stopped ){
			// If user is ready and trains are already stopped, start the finding routine. 
			notifier_msg.track = server_data.track; 
			notifier_msg.train_to_find = server_data.train_to_find;
		}
	}
}
