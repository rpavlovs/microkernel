#ifndef __NAMESERVER__
#define __NAMESERVER__

#include "userspace.h"

#define NS_ERROR_TID_IS_NOT_A_TASK			-1
#define NS_ERROR_TID_IS_NOT_A_NAMESERVER		-2
#define NS_ERROR_NOT_LEGAL_NAME				-3
#define NS_ERROR_TASK_NOT_FOUND				-4

typedef struct {
	char name[NAMESERVER_RECORD_NAME_MAX_LENGTH];
	char tid;
} ns_entry;

typedef struct {
	ns_entry entrie[NAMESERVER_TABLE_MAX_SIZE];
	int size;
} ns_table;


// Nameserver is needed by RegisterAs and WhoIs syscalls to function
// 
void nameserver();

#endif
