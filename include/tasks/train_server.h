#ifndef __TRAIN_SERVER_H__
#define __TRAIN_SERVER_H__

#include "userspace.h"

// --------------------------------------------------------------------------------------------------------------------------
// Constants
// --------------------------------------------------------------------------------------------------------------------------
#define TRAIN_REVERSE_DIRECTION				0
#define TRAIN_FORWARD_DIRECTION				1

// Train direction
#define TRAIN_DIRECTION_FORWARD				1
#define TRAIN_DIRECTION_REVERSE				2

// Train motion state
#define TRAIN_STILL							0		// The train is not moving
#define TRAIN_ACCELERATING					1		// The train has started moving and it's accelerating
#define TRAIN_CONSTANT_SPEED				2		// The train is not accelerating/deaccelerating anymore
#define TRAIN_DEACCELERATING				3		// The train is deaccelerating

// Speed type
#define	SHORT_DISTANCE_SPEED				0		// The train will move a short distance; we only care about total distance and total time
#define LONG_DISTANCE_SPEED					1		// The train will move a long distance; we also care about time accelerating/deaccelerating

// Route data
#define MAX_NUM_EDGES						100
#define MAX_NUM_LANDMARKS					100													

// Goal state
#define NO_GOAL								0		// There's no goal. The train is running freely. 
#define FIXED_GOAL							1		// A fixed goal has been established. The train is moving towars a particular position. 

// Server operation
#define TRAIN_NUM_SERVERS_TIDS				10		// The number of tids that we need to keep track of

// Train update types
#define UNDEFINED_UPDATE_TYPE				0
#define NORMAL_UPDATE_TYPE					1
#define GOAL_UPDATE_TYPE					2

// --------------------------------------------------------------------------------------------------------------------------
// Structs
// --------------------------------------------------------------------------------------------------------------------------

// -- Route data -----------------------------------------------------
typedef struct{
	// Landmarks
	int num_landmarks;							// The number of landmarks we have to traverse + 1 (the landmark where the train is). 
	int landmark_index;							// The current landmark
	track_edge *edges;
	track_node *landmarks; 
	char *switches_state; 
} Train_route_data;								// NOTE: This struct might need to be in a different file. 

typedef struct{
	// Track data
	track_node *track;
	
	// Current train information
	int *train_direction;
	track_node *train_node;
	int train_shift;

	// Route
	int *num_landmarks;							// The num. of landmarks (including the origin landmark). 
	track_node *landmarks;						// The landmarks (nodes) to follow to reach to the destination.
	track_edge *edges;							// The edges connecting the landmarks to reach the destination.

	// Current track data
	char *current_switches;
	char *target_switches;

	// Target information
	track_node *target_node;
	int target_shift;

	// Other
	int type;									// Will always be 0.
} Route_msg;									// NOTE: This struct might need to be in a different file. 

// -- Train data -----------------------------------------------------
typedef struct {
	int train_id; 
	int direction; 
	track_node *track; 
}Train_initialization_msg;

typedef struct{
	track_node *landmark;						// Which node has the train last reached. 
	track_edge *edge;							// The "path" from the last node that the train took. 
	int offset;									// The offset is described in mm.
} Train_position;

typedef struct{
	int speed_type;								// Is it a "speed" used for long or short distances? 
	int train_speed;							// The speed in train terms (0-14)

	// NOTE: THIS TWO PARAMETERS SHOULD BE REFACTORED IN OTHER STRUCT. 
	int distance_to_travel;						// The distance to travel at this speed 
	int requires_reverse;						// This flag tells if the train will perform a reverse at the end of the movement. 

	// Information for short distances
	int total_time_in_speed;					// The that the whole "trip" will take. Discovered from calibration.
	int time_accelerating;						// The time before the stop command should be executed. Discovered from calibration.
} Train_speed;									// Maybe this name should be changed to more accurately define it. 

typedef struct{
	// Data
	int train_id; 

	// State
	int direction;								// This flag tells if the train is moving forward or reverse. 
	int motion_state;
	Train_position current_position;

	// Current data
	int is_reversing;							// This flag tells  if the train is in the process of reversing. 
	int goal_state;								// This tells us if we currently have a goal or not. 
	int time_speed_change;						// The time when a command to change the speed was made. 
	Train_speed current_speed;					// The current speed at which the train is moving. 
	Train_position current_goal;				// The position where the train should move to. 
	Train_route_data route_data;				// The route to follow to reach the current goal. 

	int distance_since_speed_change;
	int distance_before_deacceleration; 

	// New data

} Train_status; 

// -- Server requests data -------------------------------------------
/*
  This struct represents the operation that the train has been requested
  to execute. The operation migth be a simple update, a request to execute
  a direct operation (rv, tr) or to move to a particular location. 
*/
typedef struct{
	int update_type;							
	Train_position *new_goal;					// The position the train should go to. 
	Command command;							// The command the train has been requested to perform.
} Train_update_request; 

// --------------------------------------------------------------------------------------------------------------------------
// Methods
// --------------------------------------------------------------------------------------------------------------------------
void train_server();

#endif
