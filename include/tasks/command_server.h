#ifndef COMMAND_SERVER_H
#define	COMMAND_SERVER_H

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define COMMAND_SERVER_NAME		"command_server"
#define COMMAND__NOTIFIER_NAME		"command_notifier"
#define COMMAND_SERVER_PRIORITY		11
#define COMMAND_NOTIFIER_PRIORITY	12

#define COMMAND_QUEUE_SIZE			100

#define ADD_CMD_REQUEST			1			
#define CMD_NOTIFIER_IDLE			2

#define TRAIN_CMD_TYPE				1
#define REVERSE_CMD_TYPE			2
#define SWITCH_CMD_TYPE				3

#define CMD_PARAM_NOT_REQUIRED		-1

#define INTER_CMD_DELAY				30		// 300 ms

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------------------------------------------------------------------------
typedef struct{
	int cmd_type; 
	int element_id;			// This represents the ID of the element on which the command will be executed, like the train or switch number. 
	int param;				// This element is used to give extra information, like speed. 
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
	int type; 
	Command cmd; 
} Cmd_request;

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
void commandserver(); 

#endif	/* COMMAND_SERVER_H */

