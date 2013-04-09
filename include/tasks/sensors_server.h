#ifndef __SENSORS_SERVER_H__
#define	__SENSORS_SERVER_H__

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define SENSOR_HISTORY_LEN			14
#define SENSORS_SERVER_NAME		"sensors_server"
#define SENSOR_QUERY_FREQENCY		5 // Queries per second 	

#define RESET_CODE					192
#define REQUEST_DATA_CODE			133

#define SENSOR_DATA_RECEIVED_MSG	1
#define WAIT_SENSOR_CHANGE_MSG		2
#define WAIT_ALL_SENSORS_CHANGE_MSG	3
#define GET_SENSOR_LIST_MSG			4
#define GET_SENSORS_CURR_VALUE		5

#define SENSOR_NAME_SIZE			4
#define NUM_SENSORS					80
#define SENSOR_WAITING_QUEUE_SIZE	10
#define NUM_TASKS_WAITING_SENSORS	10

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
} Sensor_waiting_list;

typedef struct{
	int waiting_tasks[ NUM_TASKS_WAITING_SENSORS ];
	int size; 
} All_sensors_waiting_queue;

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
	int *sensors_val_destination; 
} Sensor_msg;

typedef struct{
	int sensors_value[NUM_SENSORS];
} Sensor_update_reply;

typedef struct{
	char sensors_name[NUM_SENSORS][SENSOR_NAME_SIZE];
} Sensor_id_list_reply; 

// Server data
typedef struct {
	int curr_sensor_vals[ NUM_SENSORS ];
	Sensor_history *sensor_history; 
	Sensor_waiting_list *sensor_waiting_list; 
	All_sensors_waiting_queue *all_sensors_wait_queue; 
	Sensor_update_reply update_reply_msg; 
} Sensor_server_data;

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define GET_BIT(a,b) (int)(a & 1<<b)

void sensors_server(); 

#endif	/* SENSORS_SERVER_H */

