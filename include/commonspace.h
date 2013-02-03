#ifndef __COMMONSPACE__
#define __COMMONSPACE__

#include "helpers.h"

#include "config/mem_init.h"

#include "config/ts7200.h"

#include "lib/bwio.h"

#include "kernel/syscall.h"

// Current debug level
#define DBG_CURR_LVL 	3

// Debug levels
#define DBG_USR		 	1
#define DBG_SYS		 	2
#define DBG_KERN 		3

//
// System tasks
// 

#include "tasks/first.h"

#include "tasks/nameserver.h"


// 
// Nameserver config
// 

#define NS_TID 								1
#define NS_NAME_MAX_LENGTH 					16	//TODO: Why length is not 10?
#define NS_TABLE_MAX_SIZE					200

#define NS_REQUEST_REGISTER_AS				0
#define NS_REQUEST_WHO_IS					1

#define SUCCESS								0

#define NS_ERROR_TID_IS_NOT_A_TASK			-1
#define NS_ERROR_TID_IS_NOT_A_NAMESERVER	-2
#define NS_ERROR_NOT_LEGAL_NAME				-3
#define NS_ERROR_TASK_NOT_FOUND				-4

#endif
