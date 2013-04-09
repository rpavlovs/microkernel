#ifndef __LOC_SRV_H__
#define __LOC_SRV_H__

#include "tasks/train_data_structs.h"
#include "tasks/track_node.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define LOCATION_SERVER_NAME			"Location_Server"

// Train status
#define LC_TRAIN_STATUS_UNKNOWN			0
#define LC_TRAIN_STATUS_STOPPED			1

// Types of requests
#define LOC_SRV_FIND_TRAIN_REQ			1
#define LOC_SRV_TRAIN_STOPPED			2
#define LOC_SRV_USR_READY				3
#define LOC_SRV_TRAIN_FOUND				4

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------
typedef struct{
	int train_tid; 
	int train_num; 
	int train_status;
} Loc_srv_train_data; 

typedef struct{
	// Trains information
	Loc_srv_train_data trains_data[ NUM_TRAINS ];
	int train_to_find; 

	// Flags
	int has_find_train_request; 
	int are_trains_stopped; 
	int is_usr_ready; 

	track_node *track;

} Location_server_data ;

typedef struct{
	track_node *track; 
	int curr_sensor_vals[ NUM_SENSORS ]; 
	char sensor_names[ NUM_SENSORS ][ SENSOR_NAME_SIZE ];
} Location_notifier_data; 

// -- Messages ---------------------------------------------
typedef struct{
	int type; 
	int param; 
	int args; 
	int train_tids[ NUM_TRAINS ]; 
	int train_nums[ NUM_TRAINS ];
	track_node *train_location; 
	track_node *track;
} Location_server_msg ; 

typedef struct{
	int type; 
	Train_position train_pos; 
} Train_location_detection_msg;

typedef struct{
	int train_to_find; 
	track_node *track;
} Location_notifier_msg;

// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------
void location_detection_server();

#endif
