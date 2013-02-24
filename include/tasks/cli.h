#ifndef __TASK_CLI_H_
#define __TASK_CLI_H_ value

#define CLI_HISTORY_LENGTH 	20 // Commands

typedef struct {
	
} Command;

typedef	struct {
	char cmds[CLI_HISTORY_LENGTH];
	char resps[CLI_HISTORY_LENGTH];
	int length;
} CLI_history;

void task_cli();

#endif
