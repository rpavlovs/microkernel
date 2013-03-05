#ifndef __TASK_CLI_H__
#define __TASK_CLI_H__

#include "commonspace.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define CLI_HISTORY_LENGTH 		20	// Commands
#define CLI_COMMAND_MAX_LENGTH	20	// Characters
#define CLI_PRINT_BUFFER_SIZE		1000	// Characters

#define INVALID_TRAIN_ID			-1
#define INVALID_SPEED			-2
#define INVALID_SWITCH_ID		-3
#define INVALID_SWITCH_STATE		-4
#define INVALID_COMMAND_NAME	-5

#define CMD_SERVER_INDEX		0
#define SWITCHES_SERVER_INDEX	1

#define CLI_SERVERS_LIST_SIZE		2

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Structs
// -----------------------------------------------------------------------------------------------------------------------------------------------
typedef struct Input Input; 
typedef	struct CLI_history CLI_history;

struct Input {
	char command[UI_CLI_CMD_LINE_WIDTH];
	int status;
};

// LIFO round buffer
struct CLI_history {
	Input inputs[CLI_HISTORY_LENGTH];
	Input *head;
	Input *tail;
	int size;
};

typedef struct {
	int items[ CLI_SERVERS_LIST_SIZE ]; 
} Servers_tid_list;

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
void task_cli();

int parse_and_exec_cmd( Char_queue *buf, Servers_tid_list *servers_list );

void init_cli_history( CLI_history *h );

void update_cli_view( CLI_history *h );

void history_push( CLI_history *h, char * cmd, int status );

int exec_tr( int train_id, int speed, Servers_tid_list *servers_list );

int exec_sw( int switch_id, char state, Servers_tid_list *servers_list ); 

int exec_rv( int train_id, Servers_tid_list *servers_list );

int exec_q( );

#endif
