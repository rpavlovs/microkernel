#ifndef __TRAIN_SERVER_H__
#define __TRAIN_SERVER_H__

#include "train_data_structs.h"

// ----------------------------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------------------------

// Server operation
#define TR_CMD_SERVER_TID_INDEX					0
#define TR_SWITCH_SERVER_TID_INDEX				1
#define TR_SENSOR_SERVER_TID_INDEX				2
#define TR_ROUTE_SERVER_TID_INDEX				3
#define TR_WAIT_NOTIFIER_TID_INDEX				4
#define TR_SENSOR_NOTIFIER_TID_INDEX			5
#define TR_WAIT_NOT_COURIER_TID_INDEX			6
#define TR_SENSOR_NOT_COURIER_TID_INDEX			7
#define TR_CMD_NOT_TID_INDEX					8

// Train update types
#define NORMAL_UPDATE							1		// Request to only update the current train information
#define CHANGE_GOAL_UPDATE						2		// Request to change the goal where the train should move to
#define MOVE_FREE_UPDATE						3		// Make the train move freely
#define UPDATE_FROM_SENSOR						4		// The sensors value changed

// Train state
#define TRAIN_STATE_MOVE_FREE					1
#define TRAIN_STATE_MOVE_TO_GOAL				2

#define WAIT_NOTIFIER_DELAY						25		// 250 ms -> 4 times per second

// ----------------------------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------------------------
void train_server(); 

void initialize_train_cmd_notifier( Train_server_data *server_data );

#endif
