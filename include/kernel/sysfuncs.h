#ifndef ___SYS_FUNC___
#define ___SYS_FUNC___

#include "kernelspace.h"

// TODO: Description
//

int sys_create( int priority, void (*code) ( ), Task_descriptor *, Kern_Globals * );

// TODO: Description
//

int sys_mytid(Task_descriptor *, Kern_Globals * );

// TODO: Description
//

int sys_myparenttid(Task_descriptor *, Kern_Globals * );

// TODO: Description
//

void sys_pass(Task_descriptor *, Kern_Globals * );

// TODO: Description
//

void sys_exit(Task_descriptor *, Kern_Globals * );

// TODO: Description
//

void sys_reschedule(Task_descriptor *, Kern_Globals * );

// TODO: Description
//

int sys_send(int Tid, char *msg, int msglen, char *reply, int replylen, Task_descriptor *, Kern_Globals * );

// TODO: Description
//

int sys_receive(int *tid, char *msg, int msglen, Task_descriptor *, Kern_Globals * );

// TODO: Description
//

int sys_reply(int tid, char *reply, int replylen, Task_descriptor *, Kern_Globals * );

// TODO: Description
//

void sys_unblock_receive(Task_descriptor *td, Kern_Globals *GLOBALS );

// TODO: Description
//

int sys_await_event (int eventid, char *event, int eventLength, Task_descriptor *, Kern_Globals *);


int sys_testcall(int a, int b, int c, int d, int e, int f);

#endif
