#ifndef ___SYSCALL___
#define ___SYSCALL___

#define CREATE_SYSCALL 			0
#define MYTID_SYSCALL 			1
#define MYPARENTTID_SYSCALL 	2
#define PASS_SYSCALL 			3
#define EXIT_SYSCALL 			4
#define SEND_SYSCALL 			5
#define RECEIVE_SYSCALL 		6
#define REPLY_SYSCALL 			7
#define AWAIT_EVENT_SYSCALL		8
#define TESTCALL_SYSCALL		99

#define CREATE_ARGS 			2
#define MYTID_ARGS 				0
#define MYPARENTTID_ARGS 		0
#define PASS_ARGS 				0
#define EXIT_ARGS 				0
#define SEND_ARGS 				5
#define RECEIVE_ARGS	 		3
#define REPLY_ARGS 				3
#define AWAIT_EVENT_ARGS		1
#define TESTCALL_ARGS 			6

#define CREATE_RETURN 			1
#define MYTID_RETURN 			1
#define MYPARENTTID_RETURN 		1
#define PASS_RETURN 			0
#define EXIT_RETURN				0
#define SEND_RETURN 			1
#define RECEIVE_RETURN  		1
#define REPLY_RETURN 			1
#define AWAIT_EVENT_RETURN		1
#define TESTCALL_RETURN			1

typedef struct {

	// Additional args for create syscall
	int priority;					// fp - 24
	void (*code) ( );			// fp - 20
 
	// Place to put syscall return value (let's hope it's always integer)
	int ret; 							// fp - 16     		

} Syscall_args;


// Description:
// Create allocates and initializes a task descriptor, using the given priority,
// and the given function pointer as a pointer to the entry point of executable
// code, essentially a function with no arguments and no return value. When
// Create returns the task descriptor has all the state needed to run the task,
// the task’s stack has been suitably initialized, and the task has been entered
// into its ready queue so that it will run the next time it is scheduled.
// 
// Returns:
// • tid – the positive integer task id of the newly created task. The task id
// 		must be unique, in the sense that no task has, will have or has had the
//   	same task id.
// • -1 – if the priority is invalid.
// • -2 – if the kernel is out of task descriptors.

int Create( int priority, void (*code) ( ) );

// Description:
// Returns the task id of the calling task.
// 
// Returns:
// tid – the positive integer task id of the task that calls it.

int MyTid( );

// Description:
// Returns the task id of the task that created the calling task. This will be
// problematic only if the task has exited or been destroyed, in which case the
// return value is implementation-dependent.
// 
// Returns:
// • tid – the task id of the task that created the calling task.
// • The return value is implementation-dependent if the parent has exited, has
// 		been destroyed, or is in the process of being destroyed.

int MyParentTid( );

// Description:
// Causes a task to stop executing. The task is moved to the end of its priority
// queue, and will resume executing when next scheduled.

void Pass( );

// Description:
// Causes a task to cease execution permanently. It is removed from all priority
// queues, send queues, receive queues and awaitEvent queues. Resources owned by
// the task, primarily its memory and task descriptor are not reclaimed.
// 
// Returns:
// Exit does not return. If a point occurs where all tasks have exited the kernel
// should return cleanly to RedBoot.

void Exit( );

// Send sends a message to another task and receives a reply.
// 
// Returns:
// • The size of the message supplied by the replying task.
// • -1 – if the task id is impossible.
// • -2 – if the task id is not an existing task.
// • -3 – if the send-receive-reply transaction is incomplete.

int Send( int tid, char *msg, int msglen, char *reply, int replylen );

//  Receive blocks until a message is sent to the caller, then returns with the
//  message in its message buffer and tid set to the task id of the task that 
//  sent the message.
// 
// Returns:
// • The size of the message sent.

int Receive( int *tid, char *msg, int msglen );

// Reply sends a reply to a task that previously sent a message.
// 
// Returns:
// • 0 – if the reply succeeds.
// • -1 – if the task id is not a possible task id.
// • -2 – if the task id is not an existing task.
// • -3 – if the task is not reply blocked.

int Reply( int tid, char *reply, int replylen );

// RegisterAs registers the task id of the caller under the given name.
// 
// Returns:
//  0	- success
//  -1 	– if the nameserver task id inside the wrapper is invalid.
//  -2 	- if the nameserver task id inside the wrapper is not the nameserver.

int RegisterAs( char * );

// WhoIs asks the nameserver for the task id of the task that is registered
// under the given name.
// 
// Returns:
// tid 	– the task id of the registered task.
// -1 	– if the nameserver task id inside the wrapper is invalid.
// -2 	– if the nameserver task id inside the wrapper is not the nameserver.

int WhoIs( char * );

// Time returns the number of ticks since the clock server was created and
// initialized.
// 
// Returns:
// • non-negative integer – time in ticks since the clock server initialized.
// • -1 – if the clock server task id inside the wrapper is invalid.
// • -2 – if the clock server task id inside the wrapper is not the task id of
// 		  the clock server.

int Time();

// Delay returns after the given number of ticks has elapsed.
// 
// Returns:
// • 0 – success.
// • -1 – if the clock server task id inside the wrapper is invalid.
// • -2 – if the clock server task id inside the wrapper is not the id of the
// 		  clock server.

int Delay( int ticks );

// DelayUntil returns when the time since clock server initialization is greater
// than the given number of ticks.
// 
// Returns:
// • 0 – success.
// • -1 – if the clock server task id inside the wrapper is invalid.
// • -2 – if the clock server task id inside the wrapper is not the clock
// 		  server.

int DelayUntil( int ticks );

// AwaitEvent waits for an external event
//
// Returns:
// 0 - volatile data is in the event buffer
// -1 - invalid event
// -2 - corrupted volatile data. Error indication is in event buffer
// -3 - volatile data must be collected and interrups re-enabled in the caller
int AwaitEvent( int eventid, char *event, int eventLength );

int Getc( int channel );
int Putc( int channel, char ch );

int TestCall( int a, int b, int c, int d, int e, int f );

#endif
