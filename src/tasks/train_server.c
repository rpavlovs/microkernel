#include <userspace.h>

// -------------------------------------------------------------------
// Notifiers
// -------------------------------------------------------------------
void train_wait_notifier(){
	// Initialization 
	bwdebug( DBG_USR, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_WAIT_NOTIFIER: Enters" );

	// Variables
	int sender_tid; 
	Courier_msg courier_msg; 

	FOREVER{
		// Listening for a request
		bwdebug( DBG_USR, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_WAIT_NOTIFIER: listening for a request" );
		Receive( &sender_tid, ( char * ) &courier_msg, sizeof( courier_msg ) );

		// Delay for a certain amount of time
		Delay( WAIT_NOTIFIER_DELAY );	// 250 ms.  

		// Unblock the courier
		Reply( sender_tid, 0, 0 ); 
	}
}

void train_sensor_notifier(){
	// Initialization
	bwdebug( DBG_USR, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_SENSOR_NOTIFIER: Enters" );

	// Variables
	int sender_tid, sensor_server_tid; 

	// Messages
	Sensor_msg sensor_msg; 	
	Courier_msg courier_msg; 
	Sensor_notifier_init_msg init_msg; 
	Sensor_update_reply sensor_update_reply; 

	// Share data with the train server
	Receive( &sender_tid, ( char * ) &init_msg, sizeof( init_msg ) );
	*init_msg.sensor_state = sensor_update_reply.sensors_value;
	Reply( sender_tid, 0, 0 );

	FOREVER{
		// Listening for a request
		bwdebug( DBG_USR, TRAIN_NOTIFIERS_DEBUG_AREA, "TRAIN_SENSOR_NOTIFIER: listening for a request" );
		Receive( &sender_tid, ( char * ) &courier_msg, sizeof( courier_msg ) );

		// Wait for the sensors update
		sensor_msg.type = WAIT_ALL_SENSORS_CHANGE_MSG;
		Send( sensor_server_tid, ( char * ) &sensor_msg, sizeof( sensor_msg ), 
			( char * ) &sensor_update_reply, sizeof( sensor_update_reply ) ); 

		// Unblock the courier
		Reply( sender_tid, 0, 0 );
	}
}

// -------------------------------------------------------------------
// Helper Methods
// -------------------------------------------------------------------
/*
  This method gets the list of tids of the tasks this train server will
  be interacting with. 
  Returns:
  - An int array ( int * ) that contains the tids. 
*/
void initialize_tasks_list( Train_server_data *train_server_data ){
	// Initialization
	int *tasks_tids = train_server_data->tasks_tids;

	// Initialize all tids
	int i; 
	for( i = 0; i < NUM_REQUIRED_TASKS_FOR_TRAIN; i++ ){
		tasks_tids[i] = 0; 
	}

	// Servers
    int cmd_server_tid = WhoIs( COMMAND_SERVER_NAME ); 
    tasks_tids[ TR_CMD_SERVER_TID_INDEX ] = cmd_server_tid; 
    bwassert( cmd_server_tid >= 0, "TRAIN_SERVER: This task requires the command server to work properly." );

    int sw_server_tid = WhoIs( SWITCHES_SERVER_NAME );
    tasks_tids[ TR_SWITCH_SERVER_TID_INDEX ] = sw_server_tid; 
    bwassert( sw_server_tid >= 0, "TRAIN_SERVER: This task requires the command server to work properly." );

	int sensor_server_tid = WhoIs( SENSORS_SERVER_NAME ); 
	tasks_tids[ TR_SENSOR_SERVER_TID_INDEX ] = sensor_server_tid; 
	bwassert( sensor_server_tid >= 0, "TRAIN_SERVER: This task requires the sensor server to work properly." );

	int route_srv_tid = WhoIs( ROUTE_SERVER_NAME );
	tasks_tids[ TR_ROUTE_SERVER_TID_INDEX ] = route_srv_tid; 
	bwassert( route_srv_tid >= 0, "TRAIN_SERVER: This task requires the route server to work properly." );

	int reserv_srv_tid = WhoIs( RESERVATION_SERVER_NAME ); 
	tasks_tids[ TR_RESERVATION_SERVER_TID_INDEX ] = reserv_srv_tid; 
	bwassert( reserv_srv_tid >= 0, "TRAIN_SERVER: This task requires the reservation server to work properly." );

	// Train Cmd Notifier
	tasks_tids[TR_CMD_NOT_TID_INDEX] = Create( TRAIN_CMD_NOT_TASK_PRIORITY, train_cmd_notifier ); 
	if ( tasks_tids[TR_CMD_NOT_TID_INDEX] < 0 )
		bwassert( 0, "TRAIN_SERVER: The train requires a train command notifier to work properly." ); 
	else
		bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Train command notifier created successfully [ tid: %d ]", 
			tasks_tids[TR_CMD_NOT_TID_INDEX] );

	// WAIT Notifier
	tasks_tids[TR_WAIT_NOTIFIER_TID_INDEX] = Create( TRAIN_WAIT_NOT_TASK_PRIORITY, train_wait_notifier ); 
    if ( tasks_tids[TR_WAIT_NOTIFIER_TID_INDEX] < 0 )
		bwassert( 0, "TRAIN_SERVER: The train requires a wait notifier to work properly." ); 
	else
		bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Wait notifier created successfully [ tid: %d ]", 
			tasks_tids[TR_WAIT_NOTIFIER_TID_INDEX] );

	
	// Sensor Notifier
	/*
	Sensor_notifier_init_msg init_msg; 
	init_msg.sensor_server_tid = sensor_server_tid; 
	init_msg.sensor_state = ( int ** ) &train_server_data->notifier_sensor_values;
	tasks_tids[TR_SENSOR_NOTIFIER_TID_INDEX] = Create( TRAIN_SENSOR_NOT_TASK_PRIORITY, train_sensor_notifier ); 
    if ( tasks_tids[TR_SENSOR_NOTIFIER_TID_INDEX] < 0 ){
		bwassert( 0, "TRAIN_SERVER: The train requires a sensor notifier to work properly." ); 
	}
    else {
		Send( tasks_tids[ TR_SENSOR_NOTIFIER_TID_INDEX ], ( char * ) &init_msg, sizeof( init_msg ), 0, 0 );
		

        bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Sensor notifier created successfully [ tid: %d ]", 
			tasks_tids[TR_SENSOR_NOTIFIER_TID_INDEX] );
	}
	*/
	
	// Couriers
	int my_tid = MyTid(); 
	tasks_tids[TR_WAIT_NOT_COURIER_TID_INDEX] = create_courier( my_tid, tasks_tids[TR_WAIT_NOTIFIER_TID_INDEX], 0, 0 );
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Wait notifier courier created successfully [ tid: %d ]", 
		tasks_tids[TR_WAIT_NOT_COURIER_TID_INDEX] );

	/*
	tasks_tids[TR_SENSOR_NOT_COURIER_TID_INDEX] = create_courier( my_tid, tasks_tids[TR_SENSOR_NOTIFIER_TID_INDEX], 0, 0 );
	*/
}

void retrieve_sensor_list( Train_server_data *train_server_data ){
	// Initialization
	Sensor_msg sensor_msg;
	Sensor_id_list_reply sensor_id_list_reply;

	// Get the list of sensors
	int sensor_server_tid = train_server_data->tasks_tids[ TR_SENSOR_SERVER_TID_INDEX ];
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Retrieving sensors list [ sensor_srv_tid: %d ]", 
		sensor_server_tid );

	sensor_msg.type = GET_SENSOR_LIST_MSG; 
	Send( sensor_server_tid, ( char * ) &sensor_msg, sizeof( sensor_msg ), 
		( char * ) &sensor_id_list_reply, sizeof( sensor_id_list_reply ) );

	// Copy the values to this task's address space
	int i; 
	for( i = 0; i < NUM_SENSORS; i++ ){
		mem_cpy( sensor_id_list_reply.sensors_name[i], 
			train_server_data->sensor_names[i], SENSOR_NAME_SIZE );
	}

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Sensors retrieved succesfully [ sensor_srv_tid: %d ]", 
			sensor_server_tid );
}

void initialize_train_server_data( Train_server_data *train_server_data, Train_initialization_msg init_info ){
	// Get tasks tids
	initialize_tasks_list( train_server_data );

	// Get track data -> It's a pointer to the train manager server address space -> DON'T MODIFY!
	train_server_data->track = init_info.track; 

	// Initialize the train command notifier
	initialize_train_cmd_notifier( train_server_data );

	// Get calibration data
	load_calibration_data( &( train_server_data->calibration_data ), init_info.train_id );

	// Get sensor initialization list
	retrieve_sensor_list( train_server_data );
}

void initialize_train_cmd_notifier( Train_server_data *server_data ){
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Initializing train command notifier" );

	// The train notifier is not idle.
	server_data->is_cmd_notifier_idle = 0; 

	// Initialize the queue
	//bwprintf( COM2, "TRAIN_SERVER: Initializing train command notifier\n" );
	init_train_cmd_queue( &server_data->train_cmd_queue ); 
	//TR_CMD_NOT_TID_INDEXbwprintf( COM2, "TRAIN_SERVER: tRAIN CMD QUEUE SIZE: %d Location %d\n", server_data->train_cmd_queue.size, &server_data->train_cmd_queue.size );

	// Send the initialization info
	Train_cmd_notifier_msg msg;
	msg.queue = &( server_data->train_cmd_queue ); 
	Send( server_data->tasks_tids[TR_CMD_NOT_TID_INDEX], ( char * ) &msg, sizeof( msg ), 0, 0 );
}

void initialize_train_status( Train_status *train_status, Train_initialization_msg init_info ){
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Initializing train information [ train_id: %d ]", init_info.train_id );

	train_status->train_id = init_info.train_id; 
	train_status->train_num = init_info.train_num; 
	train_status->train_direction = init_info.direction;
	train_status->train_state = TRAIN_STATE_MOVE_FREE; 
	train_status->motion_state = TRAIN_STILL; 
	train_status->is_reversing = 0; 

	train_status->motion_data.current_error = 0;
	train_status->motion_data.train_speed = 0; 
	train_status->motion_data.original_train_speed = 0; 

	initialize_goal( train_status );

	// TODO: Temp
	if ( init_info.train_id == 49 ){
		train_status->current_position.landmark = &init_info.track[15];  // "A16"
		train_status->current_position.edge = &train_status->current_position.landmark->edge[DIR_AHEAD]; 
		//train_status->current_position.offset = 55; // 5.5 cm -> train 37
		train_status->current_position.offset = 20; // 2 cm -> train 49
	}
	else{
		train_status->current_position.landmark = &init_info.track[24];  // "B9"
		train_status->current_position.edge = &train_status->current_position.landmark->edge[DIR_AHEAD]; 
		//train_status->current_position.offset = 55; // 5.5 cm -> train 37
		train_status->current_position.offset = 130; // 13 cm -> train 50
	}
}

// -------------------------------------------------------------------
// Train Server
// -------------------------------------------------------------------
void train_server(){
	// Initialization
    bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Start execution" );
	int sender_tid, update, cmd_notifier_idle; 

	// Get initialization data
	Train_initialization_msg train_initialization; 
	Receive( &sender_tid, ( char * ) &train_initialization, sizeof( train_initialization ) );
	Reply( sender_tid, 0, 0 ); 

	// Initialize server data
	Train_server_data train_server_data;
	initialize_train_server_data( &train_server_data, train_initialization ); 
	int *tasks_tids = train_server_data.tasks_tids; 

	// Initialize train data
	Train_status train_status; 
	initialize_train_status( &train_status, train_initialization ); 
	reserve_distance( 0, &train_status, &train_server_data );	// TODO: Might need to do this somewhere else. 

	// Messages
	Courier_msg courier_msg; 
	Cmd_request cmd_request;
	Train_cmd_notifier_msg cmd_notifier_msg;
	char temp_msg_buffer[ COURIER_BUFFER_SIZE ];

	Train_update_request update_request;

	FOREVER{
		// Receive message
		bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: listening for a request" );
		Receive( &sender_tid, temp_msg_buffer, sizeof( char ) * COURIER_BUFFER_SIZE );

		// Determine how to handle the request
		update = 0; 
		cmd_notifier_idle = 0; 
		if ( sender_tid == tasks_tids[TR_WAIT_NOT_COURIER_TID_INDEX] || sender_tid == tasks_tids[TR_SENSOR_NOT_COURIER_TID_INDEX] ){
			// Server received a courier message
			mem_cpy( temp_msg_buffer, ( char * ) &courier_msg, sizeof( courier_msg ) );

			if ( sender_tid == tasks_tids[TR_SENSOR_NOT_COURIER_TID_INDEX] ){
				// Update state from sensor data. 
				bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
					"TRAIN_SERVER: Received sensor courier message [ sender_tid: %d ]", sender_tid );
				
				// If one of the triggered sensors was in the attribution list, update.
				track_node *triggered_sensor = get_sensor_triggered( &train_server_data ); 
				if ( triggered_sensor ){
					update_request.update_type = UPDATE_FROM_SENSOR; 
					update = 1; 
				}
			}
			else if( sender_tid == tasks_tids[TR_WAIT_NOT_COURIER_TID_INDEX] ){
				update_request.update_type = NORMAL_UPDATE; 
				update = 1; 
			}
			else{
				bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
					"TRAIN_SERVER: Received wait courier message [ sender_tid: %d ]", sender_tid );
			}
		}
		else if ( sender_tid == tasks_tids[TR_CMD_NOT_TID_INDEX] ){
			// Server received a message from the command notifier. It's probably idle
			mem_cpy( temp_msg_buffer, ( char * ) &cmd_notifier_msg, sizeof( cmd_notifier_msg ) ); 
			bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
				"TRAIN_SERVER: Received cmd notifier msg [ sender_tid: %d cmd_type: %d ]", sender_tid, cmd_notifier_msg.type );

			if ( cmd_notifier_msg.type == TRAIN_CMD_IDLE_MSG_TYPE ){
				// Notifier is idle. 
				train_server_data.is_cmd_notifier_idle = 1;
			}

			cmd_notifier_idle = 1; 
		}
		else {
			// Server received a command message
			mem_cpy( temp_msg_buffer, ( char * ) &cmd_request, sizeof( cmd_request ) ); 
			bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
				"TRAIN_SERVER: Received command [ sender_tid: %d cmd_type: %d ]", sender_tid, cmd_request.cmd.cmd_type );

			int cmd_type = cmd_request.cmd.cmd_type; 
			if ( cmd_type == TRAIN_CMD_TYPE || cmd_type == REVERSE_CMD_TYPE ){
				// A command was received to make the train move freely
				update_request.update_type = MOVE_FREE_UPDATE; 
				update_request.cmd = cmd_request.cmd; 
			}
			else if ( cmd_type == MOVE_TO_POSITION_CMD_TYPE ){
				// A command was made to make the train move to a particular location
				update_request.update_type = CHANGE_GOAL_UPDATE; 
				update_request.cmd = cmd_request.cmd;
			}
			else{
				bwassert( 0, "TRAIN_SERVER: The received messages must be valid. [ Sender_Tid: %d ]", sender_tid );
			}

			update = 1; 
		}
		
		// Reply the sender to unblock it. 
		if ( !cmd_notifier_idle )
			Reply( sender_tid, 0, 0 ); 

		// Should the train information be updated? 
		if ( update ){
			update_train_status( &update_request, &train_status, &train_server_data );
		}

		// Is the train notifier idle?
		if ( train_server_data.is_cmd_notifier_idle && train_server_data.train_cmd_queue.size > 0 ){
			//bwprintf( COM2, "UNBLOCKING TRAIN SERVER: train_server: idle: %d queue size: %d ", 
			//	train_server_data.is_cmd_notifier_idle, train_server_data.train_cmd_queue.size ); 
			Reply( train_server_data.tasks_tids[TR_CMD_NOT_TID_INDEX], 0, 0 );
		}
	}
}
