
// Config

#include <config/memory.h>
#include <config/ts7200.h>
#include <config/system.h>
#include <config/environment.h>
#include <config/ui.h>

// System calls

#include <kernel/syscall.h>

// Libraries

#include <lib/bwio.h>
#include <lib/io.h>
#include <lib/stdlib.h>
#include <lib/string.h>
#include <lib/queues.h>
#include <lib/helpers.h>

// System tasks 

#include "tasks/first.h"
#include "tasks/nameserver.h"
#include "tasks/timeserver.h"
#include "tasks/uart1_servers.h"
#include "tasks/uart2_servers.h"

#ifndef __COMMONSPACE__
#define __COMMONSPACE__

// General macros'

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

#define SUCCESS								0
#define FOREVER								for( ; ; )

#define CHAR_BACKSPACE						0x8
#define CHAR_NEWLINE						0xA
#define CHAR_RETURN							0xD
#define CHAR_ARROW_UP						38
#define CHAR_ARROW_DOWN						40

// Nameserver config

#define NAMESERVER_TID 						1
#define NAMESERVER_RECORD_NAME_MAX_LENGTH	30
#define NAMESERVER_TABLE_MAX_SIZE			200

//HW Events Indexes
#define TIMER1_INT_INDEX					0
#define UART1_SEND_READY					1
#define UART1_INIT_SEND						2
#define UART1_RECEIVE_READY					3
#define UART2_SEND_READY					4
#define	UART2_RECEIVE_READY					5

// Message types

#define NAMESERVER_REGISTER_AS_REQUEST 		0
#define NAMESERVER_WHO_IS_REQUEST 			1
#define NAMESERVER_REPLY		 			2

#define ERROR_WRONG_MESSAGE_TYPE		 	-10

#define UART1_RECEIVE_REQUEST				1
#define UART1_RECEIVE_REPLY					2
#define UART2_RECEIVE_REQUEST				3
#define UART2_RECEIVE_REPLY					4
#define UART1_SEND_REQUEST					5
#define UART1_SEND_REPLY					6
#define UART2_SEND_REQUEST					7
#define UART2_SEND_REPLY					8

#define UART1_RECEIVE_NOTIFIER_REQUEST		11
#define UART1_RECEIVE_NOTIFIER_REPLY		12
#define UART2_RECEIVE_NOTIFIER_REQUEST		13
#define UART2_RECEIVE_NOTIFIER_REPLY		14
#define UART1_SEND_NOTIFIER_REQUEST			15
#define UART1_SEND_NOTIFIER_REPLY			16
#define UART2_SEND_NOTIFIER_REQUEST			17
#define UART2_SEND_NOTIFIER_REPLY			18

#define INVALID_REQUEST						20

#define EVENT_UART1_SEND_READY_INITIAL		20
#define EVENT_UART1_SEND_READY				21
#define EVENT_UART1_RECEIVE_READY			22
#define EVENT_UART2_SEND_READY				23
#define EVENT_UART2_RECEIVE_READY			24

#define UART_SENDER_NOTIFIER_PRIORITY		15 // 8
#define UART_RECEIVER_NOTIFIER_PRIORITY		15 // 8


// Messages

typedef struct {
	int type;
	char ns_name[NAMESERVER_RECORD_NAME_MAX_LENGTH];
} Nameserver_request;

typedef struct {
	int type;
	int num;
} Nameserver_reply;

//IO structures
typedef struct {
	int type;
	char ch;
	//char str[2];
} UART_request;

typedef struct {
	int type;
	char ch;
	//char str[2];
} UART_reply;

#endif
