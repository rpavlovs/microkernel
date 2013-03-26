#ifndef __TRAIN_HELPERS_H__
#define __TRAIN_HELPERS_H__

#include "train_data_structs.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define TRAIN_STATUS_ROW_POS		19
#define TRAIN_STATUS_COL_POS		14

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------


// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------
int reverse_current_position( Train_status *train_status ); 

void print_train_status( Train_status *train_status ); 

void initialize_goal( Train_status *train_status ); 

void request_new_path( Train_status *train_status, Train_server_data *server_data ); 

int get_short_distance( int train_speed, Speed_calibration_data *speed_calibration );

int get_short_distance_stopping_time( int train_speed, Speed_calibration_data *speed_calibration );

int get_short_distance_total_time( int train_speed, Speed_calibration_data *speed_calibration );

#endif
