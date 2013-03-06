#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Sensors Server
// -----------------------------------------------------------------------------------------------------------------------------------------------

void init_sensor_history( Sensor_history *history ) {
	history->newest_pos = -1;
	history->size = 0;

	int i;
	for(i = 0; i < SENSOR_HISTORY_LEN; ++i){
		history->sensors[i][0] = '\0';
	}
}

void sensor_history_push( int s88_id, int pin_id, int val, Sensor_history *history ) {
	if( history->newest_pos == -1 || history->newest_pos == SENSOR_HISTORY_LEN - 1 ) {
		history->newest_pos = 0;
	} else {
		++history->newest_pos;
	}

	if( history->size < SENSOR_HISTORY_LEN ) ++history->size;

	//Perform history update
	if( val == 1 ) {
		sprintf( history->sensors[history->newest_pos], "%c%d\0",
			s88_letters[s88_id], pin_id );
	}
}

void draw_sensor_history( Sensor_history *history ) {
	char buf[100], *ptr;
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
		if( pos < 0 ) pos %= SENSOR_HISTORY_LEN;
		size += sprintf( ( ptr + size ), "%s ", history->sensors[pos] );
	}

	size += sprintf( ( ptr + size ), CURSOR_SHOW_STR );
	size += sprintf( ( ptr + size ), CURSOR_RESTORE );
	Putstr( COM2, ptr );
}

void receive_sensors( char *sensors ) {
	printf( COM2, "requesting sensors\n" );
	Putc( COM1, REQUEST_DATA_CODE );
	//Putc( COM1, RESET_CODE );
	
	int i;
	for( i = 0; i < 10; ++i ) {
		sensors[i] = Getc( COM1 );
		//printf( COM2, "recieved %d\n", sensors[i] );
	}
	printf( COM2, "Recieved sensors\n" );
}

#define GET_BIT(a,b) (int)(a & 1<<b)

void sensors_server() {
	// Data structures
	printf( COM2, "Sensors Server:\n" );
	int i, s88_num, bit_pos, val, val_prev, last_recieved;
	char s88s[10], s88s_prev[10];
	Sensor_history sensor_history;
	int my_tid = MyTid(); 
	
	// Initialization
	init_sensor_history( &sensor_history );
	int cmd_server_tid = WhoIs( COMMAND_SERVER_NAME );
	bwassert( cmd_server_tid >= 0, "SENSORS SERVER: This task requires the command server to be able to operate." ); 
	
	// Send a cmd to the command server to reset the sensors. 
	Cmd_request cmd_request; 
	cmd_request.type = QUERY_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = RESET_SENSORS_CMD_TYPE; 	
	cmd_request.cmd.sender_tid = my_tid; 
	Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  );
	
	// Receive trash. 
	cmd_request.type = QUERY_CMD_REQUEST; 
	cmd_request.cmd.cmd_type = QUERY_SENSORS_CMD_TYPE; 
	cmd_request.cmd.sensors = s88s_prev; 
	Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  ); 
	
	FOREVER {
		
		// Send the command server the request for getting the sensors. 
		cmd_request.cmd.sensors = s88s; 
		Send( cmd_server_tid, ( char * ) &cmd_request, sizeof( cmd_request ), 0, 0  ); 
		last_recieved = Time();

		// Parse sensors
		for( s88_num = 0; s88_num < 5; ++s88_num ) {
			for( bit_pos = 0; bit_pos < 8; ++bit_pos ) {
				val = GET_BIT( s88s[s88_num*2], bit_pos );
				val_prev = GET_BIT( s88s_prev[s88_num*2], bit_pos );
				if( val != val_prev )
					sensor_history_push( s88_num, bit_pos, val, &sensor_history );
			}
			for( bit_pos = 0; bit_pos < 8; ++bit_pos ) {
				val = GET_BIT( s88s[s88_num*2+1], bit_pos );
				val_prev = GET_BIT( s88s_prev[s88_num*2+1], bit_pos );
				if( val != val_prev )
					sensor_history_push( s88_num, bit_pos + 8, val, &sensor_history );
			}
		}

		// Draw the result to the screen. 
		draw_sensor_history( &sensor_history );

		// Store the previously triggered sensors to compare the next time. 
		for(i = 0; i < 10; i++) {
			s88s_prev[i] = s88s[i];
		}
		
		///DelayUntil( last_recieved + 100 / SENSOR_QUERY_FREQENCY );
		Delay( 10 ); 
	}
}
