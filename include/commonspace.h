#include "helpers.h"
#include "config/mem_init.h"
#include "config/ts7200.h"
#include "lib/bwio.h"
#include "kernel/syscall.h"

// System tasks 

#include "tasks/first.h"
#include "tasks/nameserver.h"
#include "tasks/timeserver.h"

#ifndef __COMMONSPACE__
#define __COMMONSPACE__

#define SUCCESS								0

// DEBUG PRINT

#define DBG_CURR_LVL 	3

#define DBG_FATAL	 	0	// panic and assert only
#define DBG_USR		 	1	// user tasks 
#define DBG_SYS		 	2	// sytem/user tasks
#define DBG_REQ		 	3	// kernel requests + sytem/user tasks
#define DBG_KERN 		4	// all

// Nameserver config

#define NAMESERVER_TID 						1
#define NAMESERVER_RECORD_NAME_MAX_LENGTH	30
#define NAMESERVER_TABLE_MAX_SIZE			200

// Message types

#define NAMESERVER_REGISTER_AS_REQUEST 		0
#define NAMESERVER_WHO_IS_REQUEST 			1
#define NAMESERVER_REPLY		 			2

#define ERROR_WRONG_MESSAGE_TYPE		 	-10

// Messages

typedef struct {
	int type;
	char ns_name[NAMESERVER_RECORD_NAME_MAX_LENGTH];
} Nameserver_request;

typedef struct {
	int type;
	int num;
} Nameserver_reply;

#endif
