#include <userspace.h>

// -------------------------------------------------------------------
// Calibration Data
// -------------------------------------------------------------------
int get_train_group( int train_number ){
	// TODO: Finish this list
	if ( train_number == 39 || train_number == 37 )
		return TRAIN_GROUP_1; 
	else if ( train_number == 44 || train_number == 50 )
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
			calibration_data->short_speed_cutoff = 400; // 400 cm. 

			// Speed 1
			calibration_data->speed_data[0].velocity_enabled = 0;
			calibration_data->speed_data[0].velocity = 0.0;
			calibration_data->speed_data[0].time_to_constant_speed = 0; 
			calibration_data->speed_data[0].distance_during_acceleration = 0; 
			calibration_data->speed_data[0].stopping_distance = 0; 
			calibration_data->speed_data[0].stopping_time = 0; 
			calibration_data->speed_data[0].calibrated_distances = ( int ** ) Speed_acc_table[0];

			// Speed 2
			calibration_data->speed_data[1].velocity_enabled = 0;
			calibration_data->speed_data[1].velocity = 0.0;
			calibration_data->speed_data[1].time_to_constant_speed = 0; 
			calibration_data->speed_data[1].distance_during_acceleration = 0; 
			calibration_data->speed_data[1].stopping_distance = 0; 
			calibration_data->speed_data[1].stopping_time = 0; 
			calibration_data->speed_data[1].calibrated_distances = ( int ** ) Speed_acc_table[1];

			// Speed 3
			calibration_data->speed_data[2].velocity_enabled = 0;
			calibration_data->speed_data[2].velocity = 0.0;
			calibration_data->speed_data[2].time_to_constant_speed = 0; 
			calibration_data->speed_data[2].distance_during_acceleration = 0; 
			calibration_data->speed_data[2].stopping_distance = 0; 
			calibration_data->speed_data[2].stopping_time = 0; 
			calibration_data->speed_data[2].calibrated_distances = ( int ** ) Speed_acc_table[2];

			// Speed 4
			calibration_data->speed_data[3].velocity_enabled = 0;
			calibration_data->speed_data[3].velocity = 0.0;
			calibration_data->speed_data[3].time_to_constant_speed = 0; 
			calibration_data->speed_data[3].distance_during_acceleration = 0; 
			calibration_data->speed_data[3].stopping_distance = 0; 
			calibration_data->speed_data[3].stopping_time = 0; 
			calibration_data->speed_data[3].calibrated_distances = ( int ** ) Speed_acc_table[3];

			// Speed 5
			calibration_data->speed_data[4].velocity_enabled = 0;
			calibration_data->speed_data[4].velocity = 0.161843855;
			calibration_data->speed_data[4].time_to_constant_speed = 0; 
			calibration_data->speed_data[4].distance_during_acceleration = 0; 
			calibration_data->speed_data[4].stopping_distance = 0; 
			calibration_data->speed_data[4].stopping_time = 0; 
			calibration_data->speed_data[4].calibrated_distances = ( int ** ) Speed_acc_table[4];

			// Speed 6
			calibration_data->speed_data[5].velocity_enabled = 0;
			calibration_data->speed_data[5].velocity = 0.209018338;
			calibration_data->speed_data[5].time_to_constant_speed = 0; 
			calibration_data->speed_data[5].distance_during_acceleration = 0; 
			calibration_data->speed_data[5].stopping_distance = 0; 
			calibration_data->speed_data[5].stopping_time = 0; 
			calibration_data->speed_data[5].calibrated_distances = ( int ** ) Speed_acc_table[5];

			// Speed 7
			calibration_data->speed_data[6].velocity_enabled = 0;
			calibration_data->speed_data[6].velocity = 0.259969005;
			calibration_data->speed_data[6].time_to_constant_speed = 0; 
			calibration_data->speed_data[6].distance_during_acceleration = 0; 
			calibration_data->speed_data[6].stopping_distance = 0; 
			calibration_data->speed_data[6].stopping_time = 0; 
			calibration_data->speed_data[6].calibrated_distances = ( int ** ) Speed_acc_table[6];

			// Speed 8
			calibration_data->speed_data[7].velocity_enabled = 0;
			calibration_data->speed_data[7].velocity = 0.307206565;
			calibration_data->speed_data[7].time_to_constant_speed = 0; 
			calibration_data->speed_data[7].distance_during_acceleration = 0; 
			calibration_data->speed_data[7].stopping_distance = 0; 
			calibration_data->speed_data[7].stopping_time = 0; 
			calibration_data->speed_data[7].calibrated_distances = ( int ** ) Speed_acc_table[7];

			// Speed 9
			calibration_data->speed_data[8].velocity_enabled = 0;
			calibration_data->speed_data[8].velocity = 0.349281662;
			calibration_data->speed_data[8].time_to_constant_speed = 0; 
			calibration_data->speed_data[8].distance_during_acceleration = 0; 
			calibration_data->speed_data[8].stopping_distance = 0; 
			calibration_data->speed_data[8].stopping_time = 0; 
			calibration_data->speed_data[8].calibrated_distances = ( int ** ) Speed_acc_table[8];

			// Speed 10
			calibration_data->speed_data[9].velocity_enabled = 0;
			calibration_data->speed_data[9].velocity = 0.397118511;
			calibration_data->speed_data[9].time_to_constant_speed = 0; 
			calibration_data->speed_data[9].distance_during_acceleration = 0; 
			calibration_data->speed_data[9].stopping_distance = 0; 
			calibration_data->speed_data[9].stopping_time = 0; 
			calibration_data->speed_data[9].calibrated_distances = ( int ** ) Speed_acc_table[9];

			// Speed 11
			calibration_data->speed_data[10].velocity_enabled = 0;
			calibration_data->speed_data[10].velocity = 0.437880944;
			calibration_data->speed_data[10].time_to_constant_speed = 1500; 
			calibration_data->speed_data[10].distance_during_acceleration = 0; 
			calibration_data->speed_data[10].stopping_distance = 0; 
			calibration_data->speed_data[10].stopping_time = 0; 
			calibration_data->speed_data[10].calibrated_distances = ( int ** ) Speed_acc_table[10];

			// Speed 12
			calibration_data->speed_data[11].velocity_enabled = 1;
			calibration_data->speed_data[11].velocity = 0.483845493;
			calibration_data->speed_data[11].time_to_constant_speed = 1500; 
			calibration_data->speed_data[11].distance_during_acceleration = 304;
			calibration_data->speed_data[11].stopping_distance = 896; 
			calibration_data->speed_data[11].stopping_time = 3844; // Check if it works for this train
			calibration_data->speed_data[11].calibrated_distances = ( int ** ) Speed_acc_table[11];

			// Speed 13
			calibration_data->speed_data[12].velocity_enabled = 0;
			calibration_data->speed_data[12].velocity = 0.52565354;
			calibration_data->speed_data[12].time_to_constant_speed = 0; 
			calibration_data->speed_data[12].distance_during_acceleration = 0; 
			calibration_data->speed_data[12].stopping_distance = 0; 
			calibration_data->speed_data[12].stopping_time = 0; 
			calibration_data->speed_data[12].calibrated_distances = ( int ** ) Speed_acc_table[12];

			// Speed 14
			calibration_data->speed_data[13].velocity_enabled = 0;
			calibration_data->speed_data[13].velocity = 0.574471282;
			calibration_data->speed_data[13].time_to_constant_speed = 0; 
			calibration_data->speed_data[13].distance_during_acceleration = 0; 
			calibration_data->speed_data[13].stopping_distance = 0; 
			calibration_data->speed_data[13].stopping_time = 0; 
			calibration_data->speed_data[13].calibrated_distances = ( int ** ) Speed_acc_table[13];

			break; 
		case TRAIN_GROUP_2:
			// TODO: Review this values
			calibration_data->short_speed_cutoff = 400; // 400 cm. 

			// Speed 1
			calibration_data->speed_data[0].velocity_enabled = 0;
			calibration_data->speed_data[0].velocity = 0.0;
			calibration_data->speed_data[0].time_to_constant_speed = 0; 
			calibration_data->speed_data[0].distance_during_acceleration = 0; 
			calibration_data->speed_data[0].stopping_distance = 0; 
			calibration_data->speed_data[0].stopping_time = 0; 
			calibration_data->speed_data[0].calibrated_distances = ( int ** ) Speed_acc_table_group2[0];

			// Speed 2
			calibration_data->speed_data[1].velocity_enabled = 0;
			calibration_data->speed_data[1].velocity = 0.0;
			calibration_data->speed_data[1].time_to_constant_speed = 0; 
			calibration_data->speed_data[1].distance_during_acceleration = 0; 
			calibration_data->speed_data[1].stopping_distance = 0; 
			calibration_data->speed_data[1].stopping_time = 0; 
			calibration_data->speed_data[1].calibrated_distances = ( int ** ) Speed_acc_table_group2[1];

			// Speed 3
			calibration_data->speed_data[2].velocity_enabled = 0;
			calibration_data->speed_data[2].velocity = 0.0;
			calibration_data->speed_data[2].time_to_constant_speed = 0; 
			calibration_data->speed_data[2].distance_during_acceleration = 0; 
			calibration_data->speed_data[2].stopping_distance = 0; 
			calibration_data->speed_data[2].stopping_time = 0; 
			calibration_data->speed_data[2].calibrated_distances = ( int ** ) Speed_acc_table_group2[2];

			// Speed 4
			calibration_data->speed_data[3].velocity_enabled = 0;
			calibration_data->speed_data[3].velocity = 0.0;
			calibration_data->speed_data[3].time_to_constant_speed = 0; 
			calibration_data->speed_data[3].distance_during_acceleration = 0; 
			calibration_data->speed_data[3].stopping_distance = 0; 
			calibration_data->speed_data[3].stopping_time = 0; 
			calibration_data->speed_data[3].calibrated_distances = ( int ** ) Speed_acc_table_group2[3];

			// Speed 5
			calibration_data->speed_data[4].velocity_enabled = 0;
			calibration_data->speed_data[4].velocity = 0.204075387;
			calibration_data->speed_data[4].time_to_constant_speed = 0; 
			calibration_data->speed_data[4].distance_during_acceleration = 0; 
			calibration_data->speed_data[4].stopping_distance = 0; 
			calibration_data->speed_data[4].stopping_time = 0; 
			calibration_data->speed_data[4].calibrated_distances = ( int ** ) Speed_acc_table_group2[4];

			// Speed 6
			calibration_data->speed_data[5].velocity_enabled = 0;
			calibration_data->speed_data[5].velocity = 0.252198937;
			calibration_data->speed_data[5].time_to_constant_speed = 0; 
			calibration_data->speed_data[5].distance_during_acceleration = 0; 
			calibration_data->speed_data[5].stopping_distance = 0; 
			calibration_data->speed_data[5].stopping_time = 0; 
			calibration_data->speed_data[5].calibrated_distances = ( int ** ) Speed_acc_table_group2[5];

			// Speed 7
			calibration_data->speed_data[6].velocity_enabled = 0;
			calibration_data->speed_data[6].velocity = 0.306954331;
			calibration_data->speed_data[6].time_to_constant_speed = 0; 
			calibration_data->speed_data[6].distance_during_acceleration = 0; 
			calibration_data->speed_data[6].stopping_distance = 0; 
			calibration_data->speed_data[6].stopping_time = 0; 
			calibration_data->speed_data[6].calibrated_distances = ( int ** ) Speed_acc_table_group2[6];

			// Speed 8
			calibration_data->speed_data[7].velocity_enabled = 0;
			calibration_data->speed_data[7].velocity = 0.355127702;
			calibration_data->speed_data[7].time_to_constant_speed = 0; 
			calibration_data->speed_data[7].distance_during_acceleration = 0; 
			calibration_data->speed_data[7].stopping_distance = 0; 
			calibration_data->speed_data[7].stopping_time = 0; 
			calibration_data->speed_data[7].calibrated_distances = ( int ** ) Speed_acc_table_group2[7];

			// Speed 9
			calibration_data->speed_data[8].velocity_enabled = 0;
			calibration_data->speed_data[8].velocity = 0.4051387;
			calibration_data->speed_data[8].time_to_constant_speed = 0; 
			calibration_data->speed_data[8].distance_during_acceleration = 0; 
			calibration_data->speed_data[8].stopping_distance = 0; 
			calibration_data->speed_data[8].stopping_time = 0; 
			calibration_data->speed_data[8].calibrated_distances = ( int ** ) Speed_acc_table_group2[8];

			// Speed 10
			calibration_data->speed_data[9].velocity_enabled = 0;
			calibration_data->speed_data[9].velocity = 0.447922752;
			calibration_data->speed_data[9].time_to_constant_speed = 0; 
			calibration_data->speed_data[9].distance_during_acceleration = 0; 
			calibration_data->speed_data[9].stopping_distance = 0; 
			calibration_data->speed_data[9].stopping_time = 0; 
			calibration_data->speed_data[9].calibrated_distances = ( int ** ) Speed_acc_table_group2[9];

			// Speed 11
			calibration_data->speed_data[10].velocity_enabled = 0;
			calibration_data->speed_data[10].velocity = 0.490881107;
			calibration_data->speed_data[10].time_to_constant_speed = 0; 
			calibration_data->speed_data[10].distance_during_acceleration = 0; 
			calibration_data->speed_data[10].stopping_distance = 0; 
			calibration_data->speed_data[10].stopping_time = 0; 
			calibration_data->speed_data[10].calibrated_distances = ( int ** ) Speed_acc_table_group2[10];

			// Speed 12
			calibration_data->speed_data[11].velocity_enabled = 1;
			calibration_data->speed_data[11].velocity = 0.580591343;
			calibration_data->speed_data[11].time_to_constant_speed = 3200;	// Check if this is correct 3539
			calibration_data->speed_data[11].distance_during_acceleration = 670; // Check if this is correct 
			calibration_data->speed_data[11].stopping_distance = 669; 
			calibration_data->speed_data[11].stopping_time = 3539;
			calibration_data->speed_data[11].calibrated_distances = ( int ** ) Speed_acc_table_group2[11];

			// Speed 13
			calibration_data->speed_data[12].velocity_enabled = 0;
			calibration_data->speed_data[12].velocity = 0.583022937;
			calibration_data->speed_data[12].time_to_constant_speed = 0; 
			calibration_data->speed_data[12].distance_during_acceleration = 0; 
			calibration_data->speed_data[12].stopping_distance = 0; 
			calibration_data->speed_data[12].stopping_time = 0; 
			calibration_data->speed_data[12].calibrated_distances = ( int ** ) Speed_acc_table_group2[12];

			// Speed 14
			calibration_data->speed_data[13].velocity_enabled = 0;
			calibration_data->speed_data[13].velocity = 0.583030451;
			calibration_data->speed_data[13].time_to_constant_speed = 0; 
			calibration_data->speed_data[13].distance_during_acceleration = 0; 
			calibration_data->speed_data[13].stopping_distance = 0; 
			calibration_data->speed_data[13].stopping_time = 0; 
			calibration_data->speed_data[13].calibrated_distances = ( int ** ) Speed_acc_table_group2[13];
			break; 
		case TRAIN_GROUP_3:
			// TODO: Add the calibration data: TRAIN_GROUP_3. 
			break; 
		default: 
			bwassert( 0, 
				"TRAIN_MOVEMENT_DATA: Cannot find calibration data for this train." ); 
			break; 
	}

	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, "TRAIN_SERVER: Calibration loaded successfully [ train: %d group: %d ]", 
			train_number, train_group );
}
