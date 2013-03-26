#include <userspace.h>

// ----------------------------------------------------------------------------------------------
// Train Helpers
// ----------------------------------------------------------------------------------------------
void request_new_path( Train_status *train_status, Train_server_data *server_data ){
	// TODO: Request new path. 
}

void print_train_status( Train_status *train_status ){
	// Initialization
	bwdebug( DBG_USR, TRAIN_SRV_DEBUG_AREA, 
		"TRAIN_SERVER: Printing status[ train_id: %d ]", train_status->train_id );

	char buff[500];
	char *temp_buffer = buff; 

	// Prepare the information to print
	temp_buffer += saveCursor( temp_buffer ); 
	temp_buffer += hideCursor( temp_buffer );
	temp_buffer += cursorPositioning( temp_buffer, TRAIN_STATUS_ROW_POS, TRAIN_STATUS_COL_POS );
	temp_buffer += sprintf( temp_buffer, "Id -> %d   Direction -> %s   Landmark -> %s", 
		train_status->train_id, 
		( train_status->train_direction == TRAIN_DIRECTION_FORWARD ) ? "FOR" : "REV", train_status->current_position.landmark->name ); 
	temp_buffer += append_char( temp_buffer, ' ', 5 - strlen( train_status->current_position.landmark->name ) );
	temp_buffer += sprintf( temp_buffer, 
		" Offset -> %d ", train_status->current_position.offset ); 
	temp_buffer += restoreCursor( temp_buffer );

	// Send the string to UART 2. 
	Putstr( COM2, buff );
}

/*
  This method is used to update the reference landmark when a reverse command is issued. 
*/
int reverse_current_position( Train_status *train_status ){

	// 1. Change the landmark from the source to the destination. 
	train_status->current_position.landmark = train_status->current_position.edge->dest; 
        
	// 2. Update the offset. 
	int offset = train_status->current_position.offset; 
	offset = train_status->current_position.edge->dist - offset; 
	train_status->current_position.offset = offset; 

	// 3. Change the edge. 
	// TODO: Is this the right way to do this? 
	train_status->current_position.edge = train_status->current_position.edge->reverse; 

	return 0; 
}

void initialize_goal( Train_status *train_status ){
	// If there was a goal, erase it. 
	train_status->train_state = TRAIN_STATE_MOVE_FREE; 

	// Make sure that the current "goal" is empty
	train_status->current_goal.edge = 0; 
    train_status->current_goal.landmark = 0; 
    train_status->current_goal.offset = 0; 
}

int get_short_distance( int train_speed, Speed_calibration_data *speed_calibration ){
	return speed_calibration->calibrated_distances[ train_speed ][ CALIBRATED_DISTANCE_INDEX ]; 
}

int get_short_distance_stopping_time( int train_speed, Speed_calibration_data *speed_calibration ){
	return speed_calibration->calibrated_distances[ train_speed ][ CALIBRATED_STOP_TIME_INDEX ];
}

int get_short_distance_total_time( int train_speed, Speed_calibration_data *speed_calibration ){
	return speed_calibration->calibrated_distances[ train_speed ][ CALIBRATED_TOTAL_TIME_INDEX ];
}
