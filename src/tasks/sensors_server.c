#include <userspace.h>

#define SENSOR_HISTORY_LEN 22

typedef struct {
	char commands[SENSOR_HISTORY_LEN][4];
	int newest;
} Sensor_history;

void init_sensor_history( Sensor_history *history ) {
	history->newest = -1;

	int i;
	for(i = 0; i < SENSOR_HISTORY_LEN; ++i){
		history->commands[i][0] = '\0';
	}
}

void update_sensor_history( char s80, char pin1, char pin2, Sensor_history *history ) {

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

	bwprintf( COM2, "SENSOR. Group: %c; Pin1: %c; Pin2: %c\n", s80, pin1, pin2 );
	/*
	int i;
	for (i = 0; i < SENSOR_HISTORY_LEN; ++i)
	{
		int pos = (history->newest + 1 + i) % SENSOR_HISTORY_LEN;
		bprintf( buf, "\033[s\033[%d;1H%s\033[%d;1H%s\033[u",
			SENSOR_LINE_POS - SENSOR_HISTORY_LEN + i,
			"    ",
			SENSOR_LINE_POS - SENSOR_HISTORY_LEN + i,
			history->commands[pos] );
	}*/
}

void receive_sensors(char* sensors) {
	int i;
	char reset, request;
	
	reset = 192;			
	request = 133;
	
	Putc( COM1, reset );				// NOTE: They don't add this reset message.<------------------------ 
	Putc( COM1, request );

	//bwprintf( COM2, "Get sensors started...\n");
	for( i = 0; i < 10; ++i ) {			// NOTE: This isn't consistent with either of the projects. Roman and the other dudes are reading 10. Here we're reading 9. 
		sensors[i] = Getc( COM1 );
	}
	//bwprintf( COM2, "Get sensors ended...\n");
}

void sensors_server() {
	// Data structures
	int i, j;
	int timeserver_tid;
	Msg_timeserver_request request;
	Msg_timeserver_reply reply;
	long delay_start_time, delay_end_time, delay_time;
	char s80s[5], sensors[10];
	Sensor_history sensor_history;

	// Initialization
	timeserver_tid = WhoIs("timeserver");
	request.type = TIME_REQUEST;
	for(i = 0; i < 10; i++) 
		sensors[i] = 0;
	init_sensor_history( &sensor_history );
	//TODO: refactor
	s80s[0] = 'A';
	s80s[1] = 'B';
	s80s[2] = 'C';
	s80s[3] = 'D';
	s80s[4] = 'E';

	/*
	// Getting sensors request time
	Putc( COM1, 133 );
	Send(timeserver_tid, (char *) &request, sizeof(request), (char *) &reply, sizeof(reply));
	delay_start_time = reply.num;

	// Getting sensors response	time
	Getc( COM1 );
	Send(timeserver_tid, (char *) &request, sizeof(request), (char *) &reply, sizeof(reply));
	delay_end_time = reply.num;
	delay_time = delay_end_time - delay_start_time;
	
	bwprintf( COM2, "Delay time is: %d", delay_time);

	// Receiving trash data from sensors
	for( i = 0; i < 9; ++i ) {
		Getc( COM1 );
	}
	bwprintf( COM2, "Trash received...\n");*/

	//request_sensors_data( &train_buf );
	//long wait_till = get_time() + SENSOR_CELAR_TIME;
	//while (get_time() < wait_till) try_to_recieve_char( &train_recieve_buf, COM1 );
	
	FOREVER {
		//bwprintf( COM2, "Iteration\n");
		
		receive_sensors( sensors );

		for( j = 0; j < 5; j++ ) {
			if(sensors[2*j] & 0x80) 	update_sensor_history( s80s[j], '0', '1', &sensor_history );
			if(sensors[2*j] & 0x40) 	update_sensor_history( s80s[j], '0', '2', &sensor_history );
			if(sensors[2*j] & 0x20) 	update_sensor_history( s80s[j], '0', '3', &sensor_history );
			if(sensors[2*j] & 0x10) 	update_sensor_history( s80s[j], '0', '4', &sensor_history );
			if(sensors[2*j] & 0x8)		update_sensor_history( s80s[j], '0', '5', &sensor_history );
			if(sensors[2*j] & 0x4)		update_sensor_history( s80s[j], '0', '6', &sensor_history );
			if(sensors[2*j] & 0x2)		update_sensor_history( s80s[j], '0', '7', &sensor_history );
			if(sensors[2*j] & 0x1)		update_sensor_history( s80s[j], '0', '8', &sensor_history );

			if(sensors[2*j+1] & 0x80) 	update_sensor_history( s80s[j], '0', '9', &sensor_history );
			if(sensors[2*j+1] & 0x40) 	update_sensor_history( s80s[j], '1', '0', &sensor_history );
			if(sensors[2*j+1] & 0x20) 	update_sensor_history( s80s[j], '1', '1', &sensor_history );
			if(sensors[2*j+1] & 0x10) 	update_sensor_history( s80s[j], '1', '2', &sensor_history );
			if(sensors[2*j+1] & 0x8)	update_sensor_history( s80s[j], '1', '3', &sensor_history );
			if(sensors[2*j+1] & 0x4)	update_sensor_history( s80s[j], '1', '4', &sensor_history );
			if(sensors[2*j+1] & 0x2)	update_sensor_history( s80s[j], '1', '5', &sensor_history );
			if(sensors[2*j+1] & 0x1)	update_sensor_history( s80s[j], '1', '6', &sensor_history );
		}
		
		//Delay(10);
	}
}

















