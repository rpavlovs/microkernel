#ifndef __TRAIN_STRUCT_H_
#define __TRAIN_STRUCT_H_

#include "track_node.h"
#include "track_data.h"
#include "train_movement_data.h"
#include "command_server.h"
#include "sensors_server.h"
#include "switches_server.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define NUM_REQUIRED_TASKS_FOR_TRAIN			12		// The number of tis that we need to keep track of. 

#define SENSOR_ATTR_LIST_SIZE					20		// The size of the sensor attribution list

#define RESERVATION_DISTANCE_BUFFER				0		// The extra distance to add as a safety buffer after each reservation
#define SW_CHANGE_DELAY							0		// 200 ms. 

#define TRAIN_COMMAND_QUEUE_SIZE				20		// The number of train commands waiting to be sent

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------
typedef struct Train_status_struct Train_status; 
typedef struct Train_update_request_struct Train_update_request;
//typedef struct Train_server_data_struct Train_server_data;

// -- Cmd Queue ------------------------------
typedef struct{
	int delay_until; 
	Cmd_request request; 
} Train_cmd; 

typedef struct{
	Train_cmd commands[ TRAIN_COMMAND_QUEUE_SIZE ];
	int newest, oldest; 
	int size; 
} Train_cmd_queue;

// -- Train Data ------------------------------
typedef struct{
	track_node *landmark;			// Which node has the train last reached. 
	track_edge *edge;				// The "path" from the last node that the train took. 
	int offset;						// The offset is described in mm.
} Train_position;

typedef struct{
	int calibrated_dist_index;		// Only used for short distances. 
	int train_speed;				// The current speed in terms of the "train" notation (1-14)
	int distance_type;				// The type of distance, short or long. 
	int distance_to_travel;			
	int original_train_speed;		// Only used for reversing without a goal. 
	int time_since_deacceleration; 

	// Error
	int current_error;				// The difference between the real position and the actual position.
	int total_error;				// The error since the beginning of the program. 
	int num_error_measurements;		// The number of times the error has been updated. 
} Train_motion_data; 

typedef struct{
	// Landmarks
    int num_landmarks;				// The number of landmarks we have to traverse + 1 (the landmark where the train is). 
    int landmark_index;             // The current landmark
    track_edge *edges[ TRACK_MAX ];
    track_node *landmarks[ TRACK_MAX ]; 
    char switches_state[ NUM_SWITCHES ]; 
} Train_route_data;

typedef struct{
	int distance_reserved; 
	Train_position reservation_start; 
} Train_reservation; 

struct Train_status_struct{
	// Train Info
	int train_id;
	int train_num;						// The index in the manager server (used for displaying the info about the train in the right line). 

	// Current State
	int train_state;
	int is_reversing;
	int motion_state;
	int train_direction; 
	int time_speed_change; 
	int	distance_since_speed_change;
	int distance_before_deacceleration; 

	Train_route_data route_data;
	Train_position current_goal; 
	Train_position current_position; 
	Train_motion_data motion_data; 
	Train_reservation train_reservation; 
};

struct Train_update_request_struct{
	int update_type; 
	track_node *triggered_sensor; 
	Command cmd; 
}; 

typedef struct{
	track_node *track;
	Calibration_data calibration_data; 
	int tasks_tids[ NUM_REQUIRED_TASKS_FOR_TRAIN ];

	// Train Finding State
	int is_train_finding_mode; 
	int train_mgr_id; 

	// Sensor Values ( Maybe put them in a different struct )
	int *notifier_sensor_values; 
	int sensor_values[ NUM_SENSORS ];
	char sensor_names[ NUM_SENSORS ][ SENSOR_NAME_SIZE ];

	track_node *sensor_attr_list[ SENSOR_ATTR_LIST_SIZE ];
	int num_sensors_attr_list; 

	// Command Notifier
	int is_cmd_notifier_idle; 
	Train_cmd_queue train_cmd_queue; 
} Train_server_data;

// -- Messages ------------------------------
typedef struct {
	int train_id; 
	int train_num; 
	int direction; 
	track_node *track; 
} Train_initialization_msg;

typedef struct {
	int type;
	Train_cmd_queue *queue; 
} Train_cmd_notifier_msg; 

typedef struct {
	int **sensor_state;			// A pointer to the array in the notifier's address space
	int sensor_server_tid; 
} Sensor_notifier_init_msg; 

#endif
