#include <userspace.h>

// -------------------------------------------------------------------
// Train Lost Library
// - This library contains methods that help find trains when that have
// been declared as lost. 
// -------------------------------------------------------------------
void inform_train_stopped( Train_status *train_status, Train_server_data *server_data ){
	int location_srv_tid = server_data->tasks_tids[ TR_LOC_SRV_TID_INDEX ];
	
	// Send a message to the location detection server to notify this train stopped. 
	Location_server_msg location_srv_msg; 
	location_srv_msg.type = LOC_SRV_TRAIN_STOPPED; 
	location_srv_msg.param = train_status->train_id; 
	Send( location_srv_tid, ( char * ) &location_srv_msg, sizeof( location_srv_msg ), 0, 0 ); 
}

