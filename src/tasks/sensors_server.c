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

	ptr += sprintf( ptr, CURSOR_SAVE );
	ptr += sprintf( ptr, CURSOR_HIDE_STR );
	ptr += cursorPositioning( ptr, 9, 4 );

	for( i = 0; i < history->size; ++i ) {
		if( i != 0 && i % 7 == 0 ) 
			ptr += cursorPositioning( ptr, 10, 4 );

		int pos = history->newest_pos - i;
		if( pos < 0 ) pos %= SENSOR_HISTORY_LEN;
		ptr += sprintf( ptr, "%s ", history->sensors[pos] );
	}

	ptr += sprintf( ptr, CURSOR_SHOW_STR );
	ptr += sprintf( ptr, CURSOR_RESTORE );
	Putstr( COM2, ptr );
}

void receive_sensors( char *sensors ) {
	printf( COM2, "requesting sensors\n" );
	Putc( COM1, REQUEST_DATA_CODE );
	printf( COM2, "recieving sensors\n" );

	int i;
	for( i = 0; i < 10; ++i ) {
		sensors[i] = Getc( COM1 );
		printf( COM2, "recieved %d\n", sensors[i] );
	}
	printf( COM2, "recied sensors\n" );
}

#define GET_BIT(a,b) (int)(a & 1<<b)

void sensors_server() {
	int i, s88_num, bit_pos, val, val_prev, last_recieved;
	char s88s[10], s88s_prev[10];
	Sensor_history sensor_history;

	printf( COM2, "sensors_server: enter\n" );

	// Initialization
	init_sensor_history( &sensor_history );

	// Reset sensors
	Putc( COM1, RESET_CODE );
	receive_sensors( s88s_prev );
	
	FOREVER {
		receive_sensors( s88s );
		last_recieved = Time();

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

		draw_sensor_history( &sensor_history );

		for(i = 0; i < 10; i++) {
			s88s_prev[i] = s88s[i];
		}
		DelayUntil( last_recieved + 100 / SENSOR_QUERY_FREQENCY );
	}
}
