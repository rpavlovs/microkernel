#ifndef __TRAIN_MOV_DATA_H__
#define __TRAIN_MOV_DATA_H__


// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define NUM_SPEEDS					14
#define NUM_CALIBRATED_DISTANCES	13

#define TRAIN_GROUP_1				0
#define TRAIN_GROUP_2				1
#define TRAIN_GROUP_3				2

#define MAX_SPEED					12

#define CALIBRATED_DISTANCE_INDEX	0
#define CALIBRATED_STOP_TIME_INDEX	1
#define CALIBRATED_TOTAL_TIME_INDEX	2

static const int Speed_acc_table[ 2 ][ NUM_CALIBRATED_DISTANCES ][3] = { 
	/* This array contains: 
		1. Distance traveled in a short time
		2. The moment when the stopping command was issued. 
		3. The total time the trip took. 
	*/
	// Other Speeds
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 }
	},

	// Speed 6
	{
		{ 4,   250,  260  },		// The total times are invented
		{ 8,   500,  800  },
		{ 20,  750,  1000 },
		{ 49,  1000, 1300 },
		{ 77,  1250, 1700 },
		{ 108, 1500, 2100 },
		{ 142, 1750, 3000 },
		{ 202, 2000, 3700 },
		{ 272, 2250, 4000 },
		{ 331, 2500, 5000 },
		{ 420, 2750, 5550 },
		{ 539, 3000, 5800 },
		{ 670, 3250, 6050 }
	}
}; 

static const int Speed_acc_table_group2[ 2 ][ NUM_CALIBRATED_DISTANCES ][3] = { 
	
	/* This array contains: 
		1. Distance traveled in a short time
		2. The moment when the stopping command was issued. 
		3. The total time the trip took. 
	*/
	// Other Speeds
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 }
	},
	// Speed 6
	{
		{ 2,   250,  260  },	// The total times are invented
		{ 10,  500,  800  },
		{ 28,  750,  1000 },
		{ 58,  1000, 1300 },
		{ 83,  1250, 1700 },
		{ 135, 1500, 2100 },
		{ 184, 1750, 3000 },
		{ 271, 2000, 3700 },
		{ 331, 2250, 4000 },
		{ 456, 2500, 5000 },
		{ 558, 2750, 5550 },
		{ 616, 3000, 5800 },
		{ 688, 3250, 6050 }
	}
}; 

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------
typedef struct{
	float velocity; 

	int velocity_enabled; 
	int stopping_distance; 
	int stopping_time; 

	// The time it takes to reach constant speed
	// NOTE: This is an approximation, so we might need to alter this.
	int time_to_constant_speed; 
	
	// The distance that the train travels during acceleration. 
	// NOTE: 
	// - This was measured during calibration, but it's very likely that the
	// value is not completely accurate since it contained the stopping time.
	// Further measurements might be necessary. 
	int distance_during_acceleration; 

	// This tables contain the relationship between speed (train speed, not physical speed)
	// and distance traveled during that time. 
	// NOTES: 
	// - The distance is measured in mm. 
	const int **calibrated_distances; 
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
