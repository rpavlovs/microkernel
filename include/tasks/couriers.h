#ifndef __COURIERS_H__
#define __COURIERS_H__

#include "userspace.h"

// -------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------
#define COURIER_MSG_SEND			1	// The message sent to the notifier. 
#define COURIER_MSG_REPLY			2	// The message sent to the client. 

#define COURIER_BUFFER_SIZE			500

// -------------------------------------------------------------------
// Structs
// -------------------------------------------------------------------
typedef struct{
	int message_type; 
} Courier_msg;

typedef struct{
	int client_tid; 
	int notifier_tid; 
	int message_type_client;
	int message_type_notifier;
} Courier_init_msg; 

// -------------------------------------------------------------------
// Methods
// -------------------------------------------------------------------
int create_courier( int client_id, int notifier_id, int message_type_client, int message_type_notifier );

void courier_exec();

#endif
