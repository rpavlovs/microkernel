#ifndef __TRAIN_CMD_NOT_H_
#define __TRAIN_CMD_NOT_H_

#include "train_data_structs.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define TRAIN_CMD_INIT_MSG_TYPE			1
#define	TRAIN_CMD_IDLE_MSG_TYPE			2

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------
void train_cmd_queue_push( Train_cmd cmd, Train_cmd_queue *q );

void init_train_cmd_queue( Train_cmd_queue *q ); 

void train_cmd_notifier(); 

#endif
