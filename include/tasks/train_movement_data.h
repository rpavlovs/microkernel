#ifndef __TRAIN_MOV_DATA_H__
#define __TRAIN_MOV_DATA_H__

#include "userspace.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define NUM_SPEEDS					14
#define NUM_CALIBRATED_DISTANCES	10

#define TRAIN_GROUP_1				0
#define TRAIN_GROUP_2				1
#define TRAIN_GROUP_3				2

#define MAX_SPEED					12

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------
typedef struct{
	float velocity; 

	int stopping_distance; 
	int stopping_time; 

	// The time it takes to reach constant speed
	// NOTE: This is an approximation, so we might need to alter this.
	int time_to_constant_speed; 
	
	// The distance that the train travels during acceleration. 
	int distance_during_acceleration; 

	// This tables contain the relationship between speed (train speed, not physical speed)
	// and distance traveled during that time. 
	// NOTES: 
	// - The distance is measured in mm. 
	int calibrated_distances[NUM_CALIBRATED_DISTANCES]; 
	int calibrated_distances_time[NUM_CALIBRATED_DISTANCES]; 
	int calibrated_distances_time_to_deacc[NUM_CALIBRATED_DISTANCES]; 
} Speed_calibration_data; 

typedef struct{
	Speed_calibration_data speed_data[NUM_SPEEDS]; 
	int short_speed_cutoff;		// The threshold of using short speeds. Obtained via calibration. 
} Calibration_data; 

// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------
void load_calibration_data( Calibration_data *calibration_data, int train_number );

#endif
