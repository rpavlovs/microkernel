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
// Inline Functions
// -------------------------------------------------------------------
inline int round_decimal_up( float value ); 

inline int get_next_update_time( int current_time ); 

inline int get_train_stopping_distance( int train_speed, Train_server_data *server_data );

inline int get_train_acceleration_distance( int train_speed, Train_server_data *server_data );

inline int get_reservation_distance_buffer( Train_server_data *server_data );

inline int get_sw_delay_time( Train_server_data *server_data );

inline int get_reverse_delay_time( Train_server_data *server_data );

// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------
int reverse_current_position( Train_status *train_status ); 

void print_train_status( Train_status *train_status ); 

void initialize_goal( Train_status *train_status ); 

void clear_train_motion_data( Train_status *train_status );

int request_new_path( Train_status *train_status, Train_server_data *server_data ); 

int reserve_distance( int distance_to_reserve, Train_status *train_status, Train_server_data *server_data ); 

int requires_reversing( Train_status *train_status );

track_node *get_prev_node( Train_position *train_pos );

int check_next_sw_pos( int current_speed, Train_status *train_status, Train_server_data *server_data ); 

int get_short_distance( int train_speed, Speed_calibration_data *speed_calibration );

int get_short_distance_stopping_time( int train_speed, Speed_calibration_data *speed_calibration );

int get_short_distance_total_time( int train_speed, Speed_calibration_data *speed_calibration );

int get_time_to_constant_speed( Train_status *train_status, Train_server_data *server_data ); 

int get_time_to_start_deaccelerating( Train_status *train_status, Train_server_data *server_data );

#endif
