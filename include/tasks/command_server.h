#ifndef COMMAND_SERVER_H
#define	COMMAND_SERVER_H

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define COMMAND_SERVER_NAME		"command_server"
#define COMMAND__NOTIFIER_NAME		"command_notifier"

#define COMMAND_QUEUE_SIZE			100

#define ADD_CMD_REQUEST				1			
#define CMD_NOTIFIER_IDLE			2
#define QUERY_CMD_REQUEST			3

#define TRAIN_CMD_TYPE				1
#define REVERSE_CMD_TYPE			2
#define SWITCH_CMD_TYPE				3
#define QUERY_SENSORS_CMD_TYPE		4
#define RESET_SENSORS_CMD_TYPE		5
#define SWITCH_STATE_CMD_TYPE		6
#define ALL_SWITCHES_STATE_CMD_TYPE	7
#define MOVE_TO_POSITION_CMD_TYPE	8
#define STOP_TRAIN_CMD_TYPE			9
#define RESTART_TRAIN_CMD_TYPE		10
#define UPDATE_TRAIN_POS_CMD_TYPE	11

#define CMD_PARAM_NOT_REQUIRED		-1

#define INTER_CMD_DELAY				30		// 300 ms
#define REVERSE_CMD_DELAY			200		// 2s

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------------------------------------------------------------------------
typedef struct{
	int sender_tid;
	int cmd_type; 
	int element_id;			// This represents the ID of the element on which the command will be executed, like the train or switch number. 
	int param;				// This element is used to give extra information, like speed. 
	char *sensors;			// This element is used to transfer the sensor data. 	
}Command;

typedef struct{
	Command cmds[COMMAND_QUEUE_SIZE]; 
	int size; 
	int newest, oldest; 
} Command_queue ;

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Messages
// -----------------------------------------------------------------------------------------------------------------------------------------------
typedef struct{
	Command_queue *cmd_queue; 
} Cmd_initial_msg;

typedef struct{
	Command cmd; 
	int type; 
} Cmd_request;

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
void commandserver(); 

#endif	/* COMMAND_SERVER_H */

