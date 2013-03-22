#ifndef __TRAIN_HELP_DATA_H__
#define __TRAIN_HELP_DATA_H__

#include "userspace.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
// Servers TIDs
#define TH_SWITCH_SERVER_TID_INDEX			0
#define TH_CMD_SERVER_TID_INDEX				1
#define TH_ROUTE_SERVER_TID_INDEX			2

// Train Data
#define REVERSE_DEFAULT_OFFSET				100			// TODO: Review this value. It's in mm

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------

// Speed Helpers
int get_distance_traveled( int current_time, Train_status *train_status, Calibration_data *calibration_data );

void update_train_position( int distance_since_update, Train_status *train_status, int *servers_list ); 

Train_speed calculate_speed_to_use( Train_status *status, Calibration_data *calibration_data );

int get_straight_distance( Train_status *status, int *requires_reverse );

// Path Helpers
int request_new_path( Train_status *train_status, int *servers_list, track_node *track );

int reverse_current_position( Train_status *train_status );

#endif
