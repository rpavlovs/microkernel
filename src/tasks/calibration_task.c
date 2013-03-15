#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Calibration_task
// This file contains methods used to calibrate trains. 
// NOTE: This methods are not supposed to be used during "production".
// -----------------------------------------------------------------------------------------------------------------------------------------------
int initialization(){
	// Create tasks
	int sensor_server_tid = Create( SENSOR_SERVER_PRIORITY, sensors_server );
	Create( SWITCHES_SERVER_PRIORITY, switchserver );
	
	// Clear the screen and position the cursor. 
	printf( COM2, "\033[2J\033[;H" ); 
	printf( COM2, "\033[15;2H" ); 
	
	return sensor_server_tid; 
}

void run_calibration_constant_velocity(){
	int sensor_server_tid = initialization();
	
	Sensor_msg sensor_msg; 
	sensor_msg.type = WAIT_SENSOR_CHANGE_MSG;
	
	int i; 
	for( i = 1; i < 16; i++ ){
		sensor_msg.sensor_group = 'C';
		sensor_msg.pin_id = i; 
		Send( sensor_server_tid, ( char * ) &sensor_msg, sizeof( sensor_msg ), 0, 0 ); 
		
		bwprintf( COM2, "Sensor %c%d triggered.  ", sensor_msg.sensor_group, i ); 
	}
}

void run_calibration_acceleration(){
	int sensor_server_tid = initialization();
}

void run_calibration_stopping_distance(){
	int sensor_server_tid = initialization();
}
