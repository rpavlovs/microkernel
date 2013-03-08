#ifndef __SENSORS_SERVER_H__
#define	__SENSORS_SERVER_H__

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define SENSOR_SERVER_PRIORITY		12
#define SENSOR_NOTIFIER_PRIORITY		13

#define SENSOR_HISTORY_LEN			14
#define SENSORS_SERVER_NAME		"sensors_server"
#define SENSOR_QUERY_FREQENCY		10 // Queries per second 	

#define RESET_CODE					192
#define REQUEST_DATA_CODE			133

#define SENSOR_DATA_RECEIVED_MSG	1
#define WAIT_SENSOR_CHANGE_MSG		2

#define NUM_SENSORS				80
#define SENSOR_WAITING_QUEUE_SIZE	10

static const char s88_letters[5] = { 'A', 'B', 'C', 'D', 'E' };

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
	char sensors[SENSOR_HISTORY_LEN][5];
	int newest_pos;
	int size;
} Sensor_history;

// Wake-up structure
typedef struct{
	// We are not using a circular buffer here because when we a sensor is triggered we want to 
	// inform all of the waiting tasks. However, normally just 1 task should be waiting for a particular sensor. 
	int pin_id;
	int size; 
	char s88_group;
	int waiting_tasks[ SENSOR_WAITING_QUEUE_SIZE ]; 
} Sensor_waiting_list ;

/*
typedef struct{
	int size; 
	Sensor_waiting_record records[ NUM_SENSORS ]; 
} Sensor_waiting_list; */

// Messages
typedef struct{
	int type; 
	char *sensor_data_buff; 
	char *prev_sensor_data_buff; 
} Init_sensor_msg;

typedef struct{
	int type; 
	char sensor_group; 
	int pin_id; 
} Sensor_msg;



// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define GET_BIT(a,b) (int)(a & 1<<b)

void sensors_server(); 

#endif	/* SENSORS_SERVER_H */

