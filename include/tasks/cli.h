#ifndef __TASK_CLI_H__
#define __TASK_CLI_H__

#include "commonspace.h"

#define CLI_HISTORY_LENGTH 		20	// Commands
#define CLI_COMMAND_MAX_LENGTH	10	// Characters
#define CLI_PRINT_BUFFER_SIZE	1000	// Characters

#define INVALID_TRAIN_ID		-1
#define INVALID_SPEED			-2
#define INVALID_SWITCH_ID		-3
#define INVALID_SWITCH_STATE	-4
#define INVALID_COMMAND_NAME	-5

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

void task_cli();

#endif
