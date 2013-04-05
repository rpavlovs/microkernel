#ifndef __TRAIN_MOTION_H__
#define __TRAIN_MOTION_H__

#include "train_data_structs.h"

// ----------------------------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------------------------
// Train direction
#define TRAIN_DIRECTION_FORWARD				1
#define TRAIN_DIRECTION_REVERSE				0

// Train motion state
#define TRAIN_STILL							0               // The train is not moving
#define TRAIN_ACCELERATING					1               // The train has started moving and it's accelerating
#define TRAIN_CONSTANT_SPEED				2               // The train is not accelerating/deaccelerating anymore
#define TRAIN_DEACCELERATING				3               // The train is deaccelerating

// Distance type
#define SHORT_DISTANCE						1
#define LONG_DISTANCE						2

#define TRAIN_STOP_CMD_SPEED				0

// Offset required to switch a sw safely
#define LENGTH_TRAIN_AHEAD                  50                              // TODO: Review this values
#define LENGHT_TRAIN_BACK                   150

// ----------------------------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------------------------
void update_train_status( 
	Train_update_request *update_request, Train_status *train_status, Train_server_data *server_data );

void update_train_position( 
	int current_time, Train_status *train_status, Train_server_data *server_data ); 

void handle_update_request( 
	Train_update_request *update_request, Train_status *train_status, Train_server_data *server_data ); 

track_node *get_sensor_triggered( 
	Train_server_data *server_data );

int get_short_distance_traveled( 
	int time_since_change, Train_status *train_status, Calibration_data *calibration_data );

int get_long_distance_traveled( 
	int time_since_change, Train_status *train_status, Calibration_data *calibration_data );

float get_time_for_distance( 
	float distance_in_constant_speed, int train_speed, Train_server_data *server_data );

void predict_train_movement( 
	int current_time, Train_status *train_status, Train_server_data *server_data );

void add_sensors_attrib_list( Train_status *train_status, Train_server_data *server_data );

#endif
