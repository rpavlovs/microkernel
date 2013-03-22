#include <userspace.h>

// -------------------------------------------------------------------
// Calibration Data
// -------------------------------------------------------------------
int get_train_group( int train_number ){
	// TODO: Finish this list
	if ( train_number == 39 )
		return TRAIN_GROUP_1; 
	else if ( train_number == 44 )
		return TRAIN_GROUP_2;
	else
		return -1; 
}

void load_calibration_data( Calibration_data *calibration_data, int train_number ){
	int train_group = get_train_group( train_number ); 

	switch( train_group ){
		case TRAIN_GROUP_1:
			// TODO: Add the calibration data: TRAIN_GROUP_1. 
			break; 
		case TRAIN_GROUP_2:
			// TODO: Add the calibration data: TRAIN_GROUP_2. 
			break; 
		case TRAIN_GROUP_3:
			// TODO: Add the calibration data: TRAIN_GROUP_3. 
			break; 
		default: 
			bwassert( 0, 
				"TRAIN_MOVEMENT_DATA: Cannot find calibration data for this train." ); 
			break; 
	}
}
