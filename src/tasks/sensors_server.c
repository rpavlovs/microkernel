#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Sensors Server Notifier
// -----------------------------------------------------------------------------------------------------------------------------------------------
void sensors_server_notifier(){
	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER_NOTIFIER: enters" );

	// Initialization
	Init_sensor_msg init_msg; 
	Sensor_msg sensor_msg; 
	Cmd_request cmd_request; 
	int sensor_server_tid, my_tid, last_recieved; 
	char *sensor_data; 
	
	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER_NOTIFIER: recieving init info" );
	Receive( &sensor_server_tid, (char *)&init_msg, sizeof(init_msg) );
	//Reply( sensor_server_tid, 0, 0 );
	sensor_data = init_msg.sensor_data_buff; 
	
	int cmd_server_tid = WhoIs( COMMAND_SERVER_NAME );
	bwassert( cmd_server_tid >= 0, "SENSORS SERVER: This task requires the command server to be able to operate." ); 
	
	// Reset the sensors
	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER_NOTIFIER: Resetting sensors" );
	my_tid = MyTid(); 
	cmd_request.type = QUERY_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = RESET_SENSORS_CMD_TYPE; 	
	cmd_request.cmd.sender_tid = my_tid; 
	Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  );
	
	// Receive trash?
	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER_NOTIFIER: recieving trash" );
	cmd_request.type = QUERY_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = QUERY_SENSORS_CMD_TYPE; 
	cmd_request.cmd.sensors = sensor_data; 
	Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  ); 

	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER_NOTIFIER: Unblocking sensor server" );
	Reply( sensor_server_tid, 0, 0 );
	
	// Configure the messages. 
	sensor_msg.type = SENSOR_DATA_RECEIVED_MSG; 
	
	FOREVER{
		last_recieved = Time();
		
		// Get the data from the command server
		Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  ); 
		
		// Inform the sensor server that new data arrived
		Send( sensor_server_tid, ( char * ) &sensor_msg, sizeof( sensor_msg ), 0, 0 ); 
		
		//DelayUntil( last_recieved + 100 / SENSOR_QUERY_FREQENCY );
		Delay( 20 );	// 10 
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Sensors Server
// -----------------------------------------------------------------------------------------------------------------------------------------------
// -- Sensor waiting methods ------------------------------------------------------------------------------------------------------------
void init_sensor_waiting_lists( Sensor_waiting_list *list ){
	int i; 
	for ( i = 0; i < NUM_SENSORS; i++ ){
		list->size = 0; 
		list->pin_id = i % 16; 
		list->s88_group = ( i / 16 ) + 'A'; 
		list++; 
	}
}

int get_sensor_index( char sensor_group, int pin_id ){
	return ( ( sensor_group - 'A' ) * 16 ) + pin_id; 
}

// -- Query for 1 sensor ------------------------------------
void wait_for_sensor( Sensor_waiting_list *list, int new_tid, char sensor_group, int pin_id ){
	int sensor_index = get_sensor_index( sensor_group, pin_id ); 
	Sensor_waiting_list *sensor_waiting_list = list + sensor_index; 
	
	// bwprintf( COM2, "WAITING FOR SENSOR. TID: %d GROUP: %c PIN: %d SIZE: %d\n", 
	// 		new_tid, sensor_group, pin_id, sensor_waiting_list->size ); 
	bwassert( sensor_waiting_list->size < SENSOR_WAITING_QUEUE_SIZE, 
			"Sensor waiting list should not overflow" ); 
	sensor_waiting_list->waiting_tasks[ sensor_waiting_list->size ] = new_tid; 
	sensor_waiting_list->size++; 
	
	bwdebug( DBG_USR, SENSORS_SERVER_DEBUG_AREA, "SENSOR_SERVER: task % is set to wait for sensor %c%d to be triggered.", 
			new_tid, sensor_group, pin_id ); 
}

void awaken_sensor_waiting_tasks( Sensor_waiting_list *list, char sensor_group, int pin_id ){
	int sensor_index, tid; 
	sensor_index = get_sensor_index( sensor_group, pin_id ); 
	Sensor_waiting_list *sensor_waiting_list = list + sensor_index;  

	// bwprintf( COM2, "AWAKENING. TID: GROUP: %c PIN: %d SIZE: %d INDEX: %d\n", 
	// 	sensor_group, pin_id, sensor_waiting_list->size, sensor_index ); 	
	while( sensor_waiting_list->size > 0 ){
		sensor_waiting_list->size--; 
		tid = sensor_waiting_list->waiting_tasks[ sensor_waiting_list->size ]; 

		bwdebug( DBG_USR, SENSORS_SERVER_DEBUG_AREA, "SENSOR_SERVER: Sensor  %c%d triggered. Awakening task %d", 
			sensor_group, pin_id, tid ); 
		Reply( tid, 0, 0 ); 
	}
}

// -- Query for all sensors ------------------------------------
void init_all_sensors_waiting_queue( All_sensors_waiting_queue *waiting_queue ){
	waiting_queue->size = 0; 
}

void init_id_list_reply( Sensor_id_list_reply *reply_msg ){
	int sensor_group, pin_id, sensor_index; 
	for ( sensor_group = 0; sensor_group < 5; sensor_group++ ){
		for ( pin_id = 0; pin_id < 8; pin_id++ ){
			sensor_index = get_sensor_index( sensor_group, pin_id );
			sprintf( reply_msg->sensors_name[sensor_index], "%c%d", 
				'A' + sensor_group, pin_id );
		}

		for ( pin_id = 0; pin_id < 8; pin_id++ ){
			sensor_index = get_sensor_index( sensor_group, pin_id + 8 );
			sprintf( reply_msg->sensors_name[sensor_index], "%c%d", 
				'A' + sensor_group, pin_id + 8 ); 
		}
	}
}

void wait_for_all_sensors( All_sensors_waiting_queue *waiting_queue, int tid ){
	bwassert( waiting_queue->size + 1 < NUM_TASKS_WAITING_SENSORS, 
		"SENSOR_SERVER: wait_for_all_sensors -> The waiting queue must not overflow" ); 

	waiting_queue->waiting_tasks[ waiting_queue->size++ ] = tid; 
}

void wake_all_sensors_waiting_tasks( All_sensors_waiting_queue *waiting_queue, Sensor_update_reply *reply_msg ){
	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, 
		"SENSORS_SERVER: Sensors update -> Waking up waiting tasks" );
	int i, tid; 
	for( i = 0; i < waiting_queue->size; i++ ){
		tid = waiting_queue->waiting_tasks[i];
		Reply( tid, ( char * ) reply_msg, sizeof( *reply_msg ) ); 
	}
	waiting_queue->size = 0; 
}

// -- Sensor history manipulation  methods ------------------------------------------------------------------------------------------
void init_sensor_data( char s88s[10] ){
	int i; 
	for ( i = 0; i < 10; i++ ){
		s88s[i] = '\0';
	}
}

void init_sensor_history( Sensor_history *history ) {
	history->newest_pos = -1;
	history->size = 0;

	int i;
	for(i = 0; i < SENSOR_HISTORY_LEN; ++i){
		history->sensors[i][0] = '\0';
	}
}

void sensor_history_push( int s88_id, int pin_id, int val, Sensor_history *history ) {
	//Perform history update
	if( val >= 1 ) {
		if( history->newest_pos == -1 || history->newest_pos == SENSOR_HISTORY_LEN - 1 ) {
			history->newest_pos = 0;
		} else {
			++history->newest_pos;
		}
		
		if( history->size < SENSOR_HISTORY_LEN ) 
			++history->size;
		char *str = ( pin_id > 9 ) ? "%c%d " : "%c%d  "; 

		sprintf( history->sensors[history->newest_pos], str,
			s88_letters[s88_id], pin_id );
	}
}

void draw_sensor_history( Sensor_history *history ) {
	char buf[1000], *ptr;
	int i;
	ptr = buf;
	
	int size = 0; 
	size += sprintf( ptr, CURSOR_SAVE );
	size += sprintf( ( ptr + size ), CURSOR_HIDE_STR );
	size += cursorPositioning( ( ptr + size), 9, 4 );

	for( i = 0; i < history->size; ++i ) {
		if( i != 0 && i % 7 == 0 ) 
			size += cursorPositioning( ( ptr + size ), 10, 4 );

		int pos = history->newest_pos - i;
		if( pos < 0 ) 
			pos += SENSOR_HISTORY_LEN; 
		
		size += sprintf( ( ptr + size ), "%s", history->sensors[pos] );
	}
	size += sprintf( ( ptr + size ), CURSOR_RESTORE );
	Putstr( COM2, ptr );
}

void parse_sensors( char *s88s, char *s88s_prev, Sensor_server_data *server_data ){
	// Initialization
	int s88_num, bit_pos, val, val_prev, sensor_id, send_update; 
	send_update = 0; 

	// Data structures
	Sensor_history *sensor_history = server_data->sensor_history; 
	//Sensor_waiting_list *waiting_list = server_data->sensor_waiting_list; 

	for( s88_num = 0; s88_num < 5; ++s88_num ) {
		for( bit_pos = 0; bit_pos < 8; ++bit_pos ) {
			val = GET_BIT( s88s[s88_num*2], bit_pos );
			val_prev = GET_BIT( s88s_prev[s88_num*2], bit_pos );
			if( val != val_prev ){
				sensor_id = ( bit_pos * -1 ) + 8; 
				sensor_history_push( s88_num, sensor_id, val, sensor_history );
				//awaken_sensor_waiting_tasks( waiting_list, 'A' + s88_num, sensor_id );
				if ( val ){
					// Only send an update if the sensor was triggered, not reset
					send_update = 1; 
				}
			}
		}
		for( bit_pos = 0; bit_pos < 8; ++bit_pos ) {
			val = GET_BIT( s88s[s88_num*2+1], bit_pos );
			val_prev = GET_BIT( s88s_prev[s88_num*2+1], bit_pos );
			if( val != val_prev ){
				sensor_id = ( bit_pos * -1 ) + 8; 
				sensor_history_push( s88_num, sensor_id + 8, val, sensor_history );
				//awaken_sensor_waiting_tasks( waiting_list, 'A' + s88_num, sensor_id + 8 );
				if ( val ){
					// Only send an update if the sensor was triggered, not reset
					send_update = 1; 
				}
			}
		}
	}

	// If a sensor value changed, inform the tasks waiting to be informed. 
	// TODO: Make sure that this doesn't take too long. If it does, move it to a notifier
	if ( send_update )
		wake_all_sensors_waiting_tasks( server_data->all_sensors_wait_queue, 
			server_data->update_reply_msg );
}

void store_previous_sensors( char *s88s, char *s88s_prev ){
	int i; 
	for(i = 0; i < 10; i++) {
		s88s_prev[i] = s88s[i];
	}
}

void sensors_server() {
	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER: enters" );
	RegisterAs( SENSORS_SERVER_NAME );
	
	// Data structures
	int notifier_tid, sender_tid;
	char s88s[10], s88s_prev[10];
	Sensor_history sensor_history;
	Sensor_waiting_list sensors_waiting_list[ NUM_SENSORS ];
	All_sensors_waiting_queue all_sensors_waiting_queue; 
	Sensor_server_data server_data; 
	server_data.sensor_history = &sensor_history; 
	server_data.sensor_waiting_list = sensors_waiting_list; 
	server_data.all_sensors_wait_queue = &all_sensors_waiting_queue; 

	// Messages
	Sensor_msg sensor_msg; 	
	Init_sensor_msg init_msg; 
	Sensor_id_list_reply sensor_id_list_reply; 
	
	// Initialization
	init_sensor_data( s88s ); 
	init_sensor_data( s88s_prev ); 
	init_sensor_history( &sensor_history );
	init_sensor_waiting_lists( sensors_waiting_list ); 
	init_all_sensors_waiting_queue( &all_sensors_waiting_queue ); 
	init_id_list_reply( &sensor_id_list_reply ); 

	init_msg.sensor_data_buff = s88s; 
	init_msg.prev_sensor_data_buff = s88s_prev; 
	notifier_tid = Create( SENSOR_NOTIFIER_PRIORITY, sensors_server_notifier ); 
	bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER: sensors_server_notifier created [tid: %d priority: %d]", 
			notifier_tid, SENSOR_NOTIFIER_PRIORITY );
	Send( notifier_tid, ( char * ) &init_msg, sizeof( init_msg ), 0, 0 ); 
	
	FOREVER {
		bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &sensor_msg, sizeof( sensor_msg )  );
		switch( sensor_msg.type ){
			case SENSOR_DATA_RECEIVED_MSG:				
				Reply( sender_tid, 0, 0 ); 
				parse_sensors( s88s, s88s_prev, &server_data ); // If this slows down the server put it in a different task. 
				draw_sensor_history( &sensor_history );	
				store_previous_sensors( s88s, s88s_prev );
				break; 
			case WAIT_SENSOR_CHANGE_MSG:
				// Wait for a particular sensor to be triggered
				wait_for_sensor( sensors_waiting_list, sender_tid, sensor_msg.sensor_group, sensor_msg.pin_id );
				break; 
			case WAIT_ALL_SENSORS_CHANGE_MSG:
				// Wait for any sensor to be triggered
				wait_for_all_sensors( &all_sensors_waiting_queue, sender_tid ); 
				break;
			case GET_SENSOR_LIST_MSG:
				// Get the list of sensors
				bwdebug( DBG_USR, TEMP_DEBUG_AREA, "Received msg" ); 
				Reply( sender_tid, ( char * ) &sensor_id_list_reply, sizeof( sensor_id_list_reply ) );
				bwdebug( DBG_USR, TEMP_DEBUG_AREA, "Replying msg" ); 
			default:
				bwdebug( DBG_SYS, SENSORS_SERVER_DEBUG_AREA, "SENSORS_SERVER: Invalid request. [type: %d]", sensor_msg.type );
				break; 
		}
	}
}
