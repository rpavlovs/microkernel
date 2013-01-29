#include "kernel/kernel_globals.h"

int sys_create( int priority, void (*code) ( ), Task_descriptor *, Kern_Globals * );

int sys_mytid(Task_descriptor *, Kern_Globals * );

int sys_myparenttid(Task_descriptor *, Kern_Globals * );

void sys_pass(Task_descriptor *, Kern_Globals * );

void sys_exit(Task_descriptor *, Kern_Globals * );

void sys_reschedule(Task_descriptor *, Kern_Globals * );

int sys_send(int Tid, char *msg, int msglen, char *reply, int replylen, Task_descriptor *, Kern_Globals * );

int sys_receive(int *tid, char *msg, int msglen, Task_descriptor *, Kern_Globals * );

int sys_reply(int tid, char *reply, int replylen, Task_descriptor *, Kern_Globals * );

//int sys_testcall(int a, int b, int c, int d);
//int sys_testcall(int a, int b, int c, int d, int e); //, int f);
int sys_testcall(int a, int b, int c, int d, int e, int f);
