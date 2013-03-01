#include <userspace.h>

#define SENSOR_HISTORY_LEN 22

/*
typedef struct {
	char characters[UART_BUF_SIZE];
	int newest;
	int oldest;
	int count;
} Print_buf;

typedef struct {
	char characters[UART_BUF_SIZE];
	int pos;
} Recieve_buf;

typedef struct {
	char commands[SENSOR_HISTORY_LEN][4];
	int newest;
} Sensor_history;

void init_sensor_history( Sensor_history *history ) {
	history->newest = -1;
	
	int i;
	for(i = 0; i < SENSOR_HISTORY_LEN; ++i){
		sensors->commands[i][0] = '\0';
	}
}

void update_sensor_history( Print_buf *buf, char s80, char pin1, char pin2, Sensor_history *history ) {

	//If there is nothing to update - return
	if( history->commands[history->newest][0] == s80  &&
		history->commands[history->newest][1] == pin1 &&
		history->commands[history->newest][2] == pin2 )
		return;

	//Iterate history
	if( ++(history->newest) >= SENSOR_HISTORY_LEN 
		|| history->newest == -1) {
		history->newest = 0;
	}

	//Perform history update
	history->commands[history->newest][0] = s80;
	history->commands[history->newest][1] = pin1;
	history->commands[history->newest][2] = pin2;
	history->commands[history->newest][3] = '\0';

	int i;
	for (i = 0; i < SENSOR_HISTORY_LEN; ++i)
	{
		int pos = (history->newest + 1 + i) % SENSOR_HISTORY_LEN;
		bprintf( buf, "\033[s\033[%d;1H%s\033[%d;1H%s\033[u",
			SENSOR_LINE_POS - SENSOR_HISTORY_LEN + i,
			"    ",
			SENSOR_LINE_POS - SENSOR_HISTORY_LEN + i,
			history->commands[pos] );
	}
}

void request_sensors( Print_buf *train_buf ) {
	bprintf( train_buf, "%c", 192 );
	bprintf( train_buf, "%c", 133 );
}

int sensors_recieved( Recieve_buf *train_recieve_buf ) {
	return train_recieve_buf->pos == 10;
}

void sensors_server() {
	Sensor_history sensor_history;
	init_sensor_history( &sensor_history );

	char sensors[10];

	for(i=0; i < 10; i++) {
		sensors[i] = 0;
	}

	Print_buf term_buf;
	print_buf_init(&term_buf);

	Recieve_buf term_recieve_buf;
	recieve_buf_init(&term_recieve_buf);

	// Test sensor reply delay
	long request_send_time, first_byte_recieved;
	bwprintf( COM1, "%c", 133);
	request_send_time = get_time();
	bwgetc( COM1 );
	first_byte_recieved = get_time();
	for( i = 0; i < 9; ++i ) bwgetc( COM1 );

	bprintf( &term_buf, "\033[s\033[32;1HSensor request delay using bwio: %dms\033[u",
		first_byte_recieved - request_send_time);

	for( i = 30; i < 50; ++i ) bprintf( &train_buf, "%c%c", 0, i );
	while (train_buf.oldest != -1) try_to_send_char( &train_buf, COM1 );
	
	request_sensors_data( &train_buf );
	long wait_till = get_time() + SENSOR_CELAR_TIME;
	while (get_time() < wait_till) try_to_recieve_char( &train_recieve_buf, COM1 );
	request_sensors_data( &train_buf );

	for (i = 0; i < 18; ++i)
	{
		bprintf( &train_buf, "%c%c", 34, i );
	}

	for (i = 153; i < 157; ++i)
	{
		bprintf( &train_buf, "%c%c", 34, i );
	}

	FOREVER {

		if( sensors_recieved( &train_recieve_buf ) ) {
			int j;
			for( j = 0; j < 10; j++) {
				sensors[j] = train_recieve_buf.characters[j];
			}
			clear_buffer( &train_recieve_buf );
			request_sensors_data( &train_buf );
			bprintf( &term_buf, "\033[s\033[33;1HSensor request delay in polling loop: %dms\033[u",
				get_time() - request_send_time);
			request_send_time = get_time();

			char s80s[5];
			my_strcpy(s80s, "ABCDE");

			for( j = 0; j < 5; j++ ) {
				if(sensors[2*j] & 0x80) 	update_sensor_history( &term_buf, s80s[j], '0', '1', &sensor_history ); 
				if(sensors[2*j] & 0x40) 	update_sensor_history( &term_buf, s80s[j], '0', '2', &sensor_history );
				if(sensors[2*j] & 0x20) 	update_sensor_history( &term_buf, s80s[j], '0', '3', &sensor_history ); 
				if(sensors[2*j] & 0x10) 	update_sensor_history( &term_buf, s80s[j], '0', '4', &sensor_history ); 
				if(sensors[2*j] & 0x8)		update_sensor_history( &term_buf, s80s[j], '0', '5', &sensor_history ); 
				if(sensors[2*j] & 0x4)		update_sensor_history( &term_buf, s80s[j], '0', '6', &sensor_history ); 
				if(sensors[2*j] & 0x2)		update_sensor_history( &term_buf, s80s[j], '0', '7', &sensor_history ); 
				if(sensors[2*j] & 0x1)		update_sensor_history( &term_buf, s80s[j], '0', '8', &sensor_history );
				if(sensors[2*j+1] & 0x80) 	update_sensor_history( &term_buf, s80s[j], '0', '9', &sensor_history ); 
				if(sensors[2*j+1] & 0x40) 	update_sensor_history( &term_buf, s80s[j], '1', '0', &sensor_history );
				if(sensors[2*j+1] & 0x20) 	update_sensor_history( &term_buf, s80s[j], '1', '1', &sensor_history ); 
				if(sensors[2*j+1] & 0x10) 	update_sensor_history( &term_buf, s80s[j], '1', '2', &sensor_history ); 
				if(sensors[2*j+1] & 0x8)	update_sensor_history( &term_buf, s80s[j], '1', '3', &sensor_history ); 
				if(sensors[2*j+1] & 0x4)	update_sensor_history( &term_buf, s80s[j], '1', '4', &sensor_history ); 
				if(sensors[2*j+1] & 0x2)	update_sensor_history( &term_buf, s80s[j], '1', '5', &sensor_history ); 
				if(sensors[2*j+1] & 0x1)	update_sensor_history( &term_buf, s80s[j], '1', '6', &sensor_history );  
			}
		}
	}
}
*/
















