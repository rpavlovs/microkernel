#ifndef ___SYSCALL___
#define ___SYSCALL___

#define CREATE_SYSCALL 		0
#define MYTID_SYSCALL 		1
#define MYPARENTTID_SYSCALL 	2
#define PASS_SYSCALL 		3
#define EXIT_SYSCALL 		4
#define SEND_SYSCALL 		5
#define RECEIVE_SYSCALL 	6
#define REPLY_SYSCALL 		7

#define CREATE_ARGS 		2
#define MYTID_ARGS 		0
#define MYPARENTTID_ARGS 	0
#define PASS_ARGS 		0
#define EXIT_ARGS 		0
#define SEND_ARGS 		5
#define RECEIVE_ARGS	 	3
#define REPLY_ARGS 		3

#define CREATE_RETURN 		1
#define MYTID_RETURN 		1
#define MYPARENTTID_RETURN 	1
#define PASS_RETURN 		0
#define EXIT_RETURN		0
#define SEND_RETURN 		1
#define RECEIVE_RETURN  	1
#define REPLY_RETURN 		1

typedef struct {

	// Additional args for create syscall
	int priority;					// fp - 24
	void (*code) ( );			// fp - 20
 
	// Place to put syscall return value (let's hope it's always integer)
	int ret; 							// fp - 16     		

} Syscall_args;

// Name:
// Create - instantiate a task.
// 
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

// Name:
// MyTid - find my task id.
// 
// Description:
// Returns the task id of the calling task.
// 
// Returns:
// tid – the positive integer task id of the task that calls it.
int MyTid( );

// Name:
// MyParentTid - find the task id of the task that created the running task.
// 
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

// Name:
// Pass - cease execution, remaining ready to run.
// 
// Description:
// Causes a task to stop executing. The task is moved to the end of its priority
// queue, and will resume executing when next scheduled.
void Pass( );

// Name:
// Exit - terminate execution forever.
// 
// Description:
// Causes a task to cease execution permanently. It is removed from all priority
// queues, send queues, receive queues and awaitEvent queues. Resources owned by
// the task, primarily its memory and task descriptor are not reclaimed.
// 
// Returns:
// Exit does not return. If a point occurs where all tasks have exited the kernel
// should return cleanly to RedBoot.
void Exit( );


int Send(int Tid, char *msg, int msglen, char *reply, int replylen);

int Receive(int *tid, char *msg, int msglen);

int Reply(int tid, char *reply, int replylen);












#endif
