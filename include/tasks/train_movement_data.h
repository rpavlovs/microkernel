#ifndef __TRAIN_MOV_DATA_H__
#define __TRAIN_MOV_DATA_H__


// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define NUM_SPEEDS					14
#define NUM_CALIBRATED_DISTANCES	5

#define TRAIN_GROUP_1				0
#define TRAIN_GROUP_2				1
#define TRAIN_GROUP_3				2

#define MAX_SPEED					12

#define CALIBRATED_DISTANCE_INDEX	0
#define CALIBRATED_STOP_TIME_INDEX	1
#define CALIBRATED_TOTAL_TIME_INDEX	2

static const int Speed_acc_table[ NUM_SPEEDS ][ NUM_CALIBRATED_DISTANCES ][3] = { 
	/* This array contains: 
		1. Distance traveled in a short time
		2. The moment when the stopping command was issued. 
		3. The total time the trip took. 
	*/

	// Speed 1
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	}, 

	// Speed 2
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 3
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 4
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 5
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 6
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 7
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 8
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 9
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 10
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 11
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 12
	{
		{ 14,  500,  520  },	// The stopping time is invented
		{ 117, 1000, 1200 },	// The stopping time is invented
		{ 304, 1500, 2000 },	// The stopping time is invented
		{ 610, 2000, 2500 },	// The stopping time is invented
		{ 987, 2500, 3000 },	// The stopping time is invented
	},

	// Speed 13
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 14
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	}
}; 

static const int Speed_acc_table_group2[ NUM_SPEEDS ][ NUM_CALIBRATED_DISTANCES ][3] = { 
	/* This array contains: 
		1. Distance traveled in a short time
		2. The moment when the stopping command was issued. 
		3. The total time the trip took. 
	*/

	// Speed 1
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	}, 

	// Speed 2
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 3
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 4
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 5
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 6
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 7
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 8
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 9
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 10
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 11
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 12
	{
		{ 0, 1000, 0 },	// The stopping time is invented
		{ 0, 2000, 0 },	// The stopping time is invented
		{ 0, 3000, 0 },	// The stopping time is invented
		{ 0, 4000, 0 },	// The stopping time is invented
		{ 0, 5000, 0 },	// The stopping time is invented
	},

	// Speed 13
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	},

	// Speed 14
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
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
	int **calibrated_distances; 
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
