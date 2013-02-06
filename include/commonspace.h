
// Config

#include <config/memory.h>
#include <config/ts7200.h>
#include <config/system.h>

// System calls

#include <kernel/syscall.h>

// Libraries

#include <lib/bwio.h>
#include <lib/stdlib.h>
#include <lib/string.h>

// System tasks 

#include "tasks/first.h"
#include "tasks/nameserver.h"
#include "tasks/timeserver.h"

#ifndef __COMMONSPACE__
#define __COMMONSPACE__

#define SUCCESS								0
#define FOREVER								for( ; ; )

// Nameserver config

#define NAMESERVER_TID 						1
#define NAMESERVER_RECORD_NAME_MAX_LENGTH	30
#define NAMESERVER_TABLE_MAX_SIZE			200

//HW Events Indexes
#define TIMER1_INT_INDEX					0

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
