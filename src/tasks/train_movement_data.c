#include <userspace.h>

// -------------------------------------------------------------------
// Calibration Data
// -------------------------------------------------------------------
int get_train_group( int train_number ){
	// TODO: Finish this list
	if ( train_number == 45 || train_number == 48 )
		return TRAIN_GROUP_1; 
	else if ( train_number == 44 || train_number == 49 || train_number == 50 )
		return TRAIN_GROUP_2;
	else
		return -1; 
}

void load_calibration_data( Calibration_data *calibration_data, int train_number ){
	int train_group = get_train_group( train_number ); 
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Loading calibration data [ train: %d group: %d ]", 
			train_number, train_group );

	switch( train_group ){
		case TRAIN_GROUP_1:
			// TODO: Review this values
			calibration_data->short_speed_cutoff = 671; // 67.1 cm. 

			// Speed 1
			calibration_data->speed_data[0].velocity_enabled = 0;
			calibration_data->speed_data[0].velocity = 0.0;
			calibration_data->speed_data[0].time_to_constant_speed = 0; 
			calibration_data->speed_data[0].distance_during_acceleration = 0; 
			calibration_data->speed_data[0].stopping_distance = 0; 
			calibration_data->speed_data[0].stopping_time = 0; 
			calibration_data->speed_data[0].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 2
			calibration_data->speed_data[1].velocity_enabled = 0;
			calibration_data->speed_data[1].velocity = 0.0;
			calibration_data->speed_data[1].time_to_constant_speed = 0; 
			calibration_data->speed_data[1].distance_during_acceleration = 0; 
			calibration_data->speed_data[1].stopping_distance = 0; 
			calibration_data->speed_data[1].stopping_time = 0; 
			calibration_data->speed_data[1].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 3
			calibration_data->speed_data[2].velocity_enabled = 0;
			calibration_data->speed_data[2].velocity = 0.0;
			calibration_data->speed_data[2].time_to_constant_speed = 0; 
			calibration_data->speed_data[2].distance_during_acceleration = 0; 
			calibration_data->speed_data[2].stopping_distance = 0; 
			calibration_data->speed_data[2].stopping_time = 0; 
			calibration_data->speed_data[2].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 4
			calibration_data->speed_data[3].velocity_enabled = 0;
			calibration_data->speed_data[3].velocity = 0.0;
			calibration_data->speed_data[3].time_to_constant_speed = 0; 
			calibration_data->speed_data[3].distance_during_acceleration = 0; 
			calibration_data->speed_data[3].stopping_distance = 0; 
			calibration_data->speed_data[3].stopping_time = 0; 
			calibration_data->speed_data[3].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 5
			calibration_data->speed_data[4].velocity_enabled = 0;
			calibration_data->speed_data[4].velocity = 0.161843855;
			calibration_data->speed_data[4].time_to_constant_speed = 0; 
			calibration_data->speed_data[4].distance_during_acceleration = 0; 
			calibration_data->speed_data[4].stopping_distance = 0; 
			calibration_data->speed_data[4].stopping_time = 0; 
			calibration_data->speed_data[4].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 6
			calibration_data->speed_data[5].velocity_enabled = 0;
			calibration_data->speed_data[5].velocity = 0.277523037;
			calibration_data->speed_data[5].time_to_constant_speed = 3250;	
			calibration_data->speed_data[5].distance_during_acceleration = 292;		
			calibration_data->speed_data[5].stopping_distance = 379; 
			calibration_data->speed_data[5].stopping_time = 0;					// TODO: Add the stopping time. 
			calibration_data->speed_data[5].calibrated_distances = ( const int ** ) Speed_acc_table[1];

			// Speed 7
			calibration_data->speed_data[6].velocity_enabled = 0;
			calibration_data->speed_data[6].velocity = 0.259969005;
			calibration_data->speed_data[6].time_to_constant_speed = 0; 
			calibration_data->speed_data[6].distance_during_acceleration = 0; 
			calibration_data->speed_data[6].stopping_distance = 0; 
			calibration_data->speed_data[6].stopping_time = 0; 
			calibration_data->speed_data[6].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 8
			calibration_data->speed_data[7].velocity_enabled = 0;
			calibration_data->speed_data[7].velocity = 0.307206565;
			calibration_data->speed_data[7].time_to_constant_speed = 0; 
			calibration_data->speed_data[7].distance_during_acceleration = 0; 
			calibration_data->speed_data[7].stopping_distance = 0; 
			calibration_data->speed_data[7].stopping_time = 0; 
			calibration_data->speed_data[7].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 9
			calibration_data->speed_data[8].velocity_enabled = 0;
			calibration_data->speed_data[8].velocity = 0.407371567;
			calibration_data->speed_data[8].time_to_constant_speed = 0;			// TODO: Add the time to constant speed
			calibration_data->speed_data[8].distance_during_acceleration = 0;	// TODO: Add the distance during acceleration
			calibration_data->speed_data[8].stopping_distance = 578; 
			calibration_data->speed_data[8].stopping_time = 0;					// TODO: Add the stopping time. 
			calibration_data->speed_data[8].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 10
			calibration_data->speed_data[9].velocity_enabled = 0;
			calibration_data->speed_data[9].velocity = 0.397118511;
			calibration_data->speed_data[9].time_to_constant_speed = 0; 
			calibration_data->speed_data[9].distance_during_acceleration = 0; 
			calibration_data->speed_data[9].stopping_distance = 0; 
			calibration_data->speed_data[9].stopping_time = 0; 
			calibration_data->speed_data[9].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 11
			calibration_data->speed_data[10].velocity_enabled = 0;
			calibration_data->speed_data[10].velocity = 0.437880944;
			calibration_data->speed_data[10].time_to_constant_speed = 1500; 
			calibration_data->speed_data[10].distance_during_acceleration = 0; 
			calibration_data->speed_data[10].stopping_distance = 0; 
			calibration_data->speed_data[10].stopping_time = 0; 
			calibration_data->speed_data[10].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 12
			calibration_data->speed_data[11].velocity_enabled = 1;
			calibration_data->speed_data[11].velocity = 0.542435002;
			calibration_data->speed_data[11].time_to_constant_speed = 0;		// TODO: Add the time to constant speed
			calibration_data->speed_data[11].distance_during_acceleration = 0;	// TODO: Add the distance during acceleration
			calibration_data->speed_data[11].stopping_distance = 812;			
			calibration_data->speed_data[11].stopping_time = 0;					// TODO: Add the stopping time. 
			calibration_data->speed_data[11].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 13
			calibration_data->speed_data[12].velocity_enabled = 0;
			calibration_data->speed_data[12].velocity = 0.52565354;
			calibration_data->speed_data[12].time_to_constant_speed = 0; 
			calibration_data->speed_data[12].distance_during_acceleration = 0; 
			calibration_data->speed_data[12].stopping_distance = 0; 
			calibration_data->speed_data[12].stopping_time = 0; 
			calibration_data->speed_data[12].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			// Speed 14
			calibration_data->speed_data[13].velocity_enabled = 0;
			calibration_data->speed_data[13].velocity = 0.574471282;
			calibration_data->speed_data[13].time_to_constant_speed = 0; 
			calibration_data->speed_data[13].distance_during_acceleration = 0; 
			calibration_data->speed_data[13].stopping_distance = 0; 
			calibration_data->speed_data[13].stopping_time = 0; 
			calibration_data->speed_data[13].calibrated_distances = ( const int ** ) Speed_acc_table[0];

			break; 
		case TRAIN_GROUP_2:
			// TODO: Review this values
			calibration_data->short_speed_cutoff = 616; // 61.6 cm. 

			// Speed 1
			calibration_data->speed_data[0].velocity_enabled = 0;
			calibration_data->speed_data[0].velocity = 0.0;
			calibration_data->speed_data[0].time_to_constant_speed = 0; 
			calibration_data->speed_data[0].distance_during_acceleration = 0; 
			calibration_data->speed_data[0].stopping_distance = 0; 
			calibration_data->speed_data[0].stopping_time = 0; 
			calibration_data->speed_data[0].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 2
			calibration_data->speed_data[1].velocity_enabled = 0;
			calibration_data->speed_data[1].velocity = 0.0;
			calibration_data->speed_data[1].time_to_constant_speed = 0; 
			calibration_data->speed_data[1].distance_during_acceleration = 0; 
			calibration_data->speed_data[1].stopping_distance = 0; 
			calibration_data->speed_data[1].stopping_time = 0; 
			calibration_data->speed_data[1].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 3
			calibration_data->speed_data[2].velocity_enabled = 0;
			calibration_data->speed_data[2].velocity = 0.0;
			calibration_data->speed_data[2].time_to_constant_speed = 0; 
			calibration_data->speed_data[2].distance_during_acceleration = 0; 
			calibration_data->speed_data[2].stopping_distance = 0; 
			calibration_data->speed_data[2].stopping_time = 0; 
			calibration_data->speed_data[2].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 4
			calibration_data->speed_data[3].velocity_enabled = 0;
			calibration_data->speed_data[3].velocity = 0.0;
			calibration_data->speed_data[3].time_to_constant_speed = 0; 
			calibration_data->speed_data[3].distance_during_acceleration = 0; 
			calibration_data->speed_data[3].stopping_distance = 0; 
			calibration_data->speed_data[3].stopping_time = 0; 
			calibration_data->speed_data[3].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 5
			calibration_data->speed_data[4].velocity_enabled = 0;
			calibration_data->speed_data[4].velocity = 0.204075387;
			calibration_data->speed_data[4].time_to_constant_speed = 0; 
			calibration_data->speed_data[4].distance_during_acceleration = 0; 
			calibration_data->speed_data[4].stopping_distance = 0; 
			calibration_data->speed_data[4].stopping_time = 0; 
			calibration_data->speed_data[4].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 6
			calibration_data->speed_data[5].velocity_enabled = 1;
			calibration_data->speed_data[5].velocity = 0.271970398;
			//calibration_data->speed_data[5].time_to_constant_speed = 2804;			// Check if this is correct 
			//calibration_data->speed_data[5].distance_during_acceleration = 299;		// Check if this is correct
			calibration_data->speed_data[5].time_to_constant_speed = 2804;			// Check if this is correct 
			calibration_data->speed_data[5].distance_during_acceleration = 355;		// Check if this is correct
			calibration_data->speed_data[5].stopping_distance = 299; 
			calibration_data->speed_data[5].stopping_time = 2804; 
			calibration_data->speed_data[5].calibrated_distances = ( const int ** ) Speed_acc_table_group2[1];

			// Speed 7
			calibration_data->speed_data[6].velocity_enabled = 0;
			calibration_data->speed_data[6].velocity = 0.306954331;
			calibration_data->speed_data[6].time_to_constant_speed = 0; 
			calibration_data->speed_data[6].distance_during_acceleration = 0; 
			calibration_data->speed_data[6].stopping_distance = 0; 
			calibration_data->speed_data[6].stopping_time = 0; 
			calibration_data->speed_data[6].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 8
			calibration_data->speed_data[7].velocity_enabled = 0;
			calibration_data->speed_data[7].velocity = 0.355127702;
			calibration_data->speed_data[7].time_to_constant_speed = 0; 
			calibration_data->speed_data[7].distance_during_acceleration = 0; 
			calibration_data->speed_data[7].stopping_distance = 0; 
			calibration_data->speed_data[7].stopping_time = 0; 
			calibration_data->speed_data[7].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 9
			calibration_data->speed_data[8].velocity_enabled = 1;
			calibration_data->speed_data[8].velocity = 0.437856328;
			//calibration_data->speed_data[8].time_to_constant_speed = 3000;			// Check if this is correct 
			//calibration_data->speed_data[8].distance_during_acceleration = 481;		// Check if this is correct 
			calibration_data->speed_data[8].time_to_constant_speed = 3000;			// Check if this is correct 
			calibration_data->speed_data[8].distance_during_acceleration = 430;		// Check if this is correct 
			calibration_data->speed_data[8].stopping_distance = 481; 
			calibration_data->speed_data[8].stopping_time = 3415; 
			calibration_data->speed_data[8].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 10
			calibration_data->speed_data[9].velocity_enabled = 0;
			calibration_data->speed_data[9].velocity = 0.447922752;
			calibration_data->speed_data[9].time_to_constant_speed = 0; 
			calibration_data->speed_data[9].distance_during_acceleration = 0; 
			calibration_data->speed_data[9].stopping_distance = 0; 
			calibration_data->speed_data[9].stopping_time = 0; 
			calibration_data->speed_data[9].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 11
			calibration_data->speed_data[10].velocity_enabled = 0;
			calibration_data->speed_data[10].velocity = 0.490881107;
			calibration_data->speed_data[10].time_to_constant_speed = 0; 
			calibration_data->speed_data[10].distance_during_acceleration = 0; 
			calibration_data->speed_data[10].stopping_distance = 0; 
			calibration_data->speed_data[10].stopping_time = 0; 
			calibration_data->speed_data[10].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 12
			calibration_data->speed_data[11].velocity_enabled = 1;
			calibration_data->speed_data[11].velocity = 0.581332839;
			//calibration_data->speed_data[11].time_to_constant_speed = 3200;	// Check if this is correct 3539
			calibration_data->speed_data[11].time_to_constant_speed = 3545;	// Check if this is correct 3539
			//calibration_data->speed_data[11].distance_during_acceleration = 670; // Check if this is correct 
			calibration_data->speed_data[11].distance_during_acceleration = 700; // Check if this is correct 
			calibration_data->speed_data[11].stopping_distance = 669; 
			calibration_data->speed_data[11].stopping_time = 3539;
			calibration_data->speed_data[11].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 13
			calibration_data->speed_data[12].velocity_enabled = 0;
			calibration_data->speed_data[12].velocity = 0.583022937;
			calibration_data->speed_data[12].time_to_constant_speed = 0; 
			calibration_data->speed_data[12].distance_during_acceleration = 0; 
			calibration_data->speed_data[12].stopping_distance = 0; 
			calibration_data->speed_data[12].stopping_time = 0; 
			calibration_data->speed_data[12].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];

			// Speed 14
			calibration_data->speed_data[13].velocity_enabled = 0;
			calibration_data->speed_data[13].velocity = 0.583030451;
			calibration_data->speed_data[13].time_to_constant_speed = 0; 
			calibration_data->speed_data[13].distance_during_acceleration = 0; 
			calibration_data->speed_data[13].stopping_distance = 0; 
			calibration_data->speed_data[13].stopping_time = 0; 
			calibration_data->speed_data[13].calibrated_distances = ( const int ** ) Speed_acc_table_group2[0];
			break; 
		case TRAIN_GROUP_3:
			// TODO: Add the calibration data: TRAIN_GROUP_3. 
			break; 
		default: 
			bwassert( 0, 
				"TRAIN_MOVEMENT_DATA: Cannot find calibration data for this train. [ train_id: %d ]", 
				train_number ); 
			break; 
	}

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Calibration loaded successfully [ train: %d group: %d ]", 
			train_number, train_group );
}
