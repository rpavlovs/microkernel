#ifndef TRAIN_MANAGER_H
#define	TRAIN_MANAGER_H

#include "userspace.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
// Track data
#define TRACK_ID_A							1
#define TRACK_ID_B							2

// Message types
#define TRAIN_MGR_INITIALIZATION_MSG		1
#define TRAIN_MGR_ADD_TRAIN_MSG				2
#define TRAIN_MGR_FIND_TRAIN_MSG			3

#define NUM_TRAINS							3

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------
typedef struct{
	int track_id; 
}Train_mgr_init_msg; 

typedef struct{
	track_node *track;
}Train_mgr_init_reply; 

typedef struct{
	int msg_type; 
	int param;
	int element_id; 
}Train_manager_msg;

typedef struct{
	int train_tid; 
}Train_manager_reply; 

// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------
void train_manager(); 

track_node *get_location_node( const char *location_name, track_node *track );

#endif
