#include "kernel/kernel_globals.h"

int sys_create( int priority, void (*code) ( ), Kern_Globals * );

int sys_mytid(Task_descriptor *, Kern_Globals * );

int sys_myparenttid(Task_descriptor *, Kern_Globals * );

void sys_pass(Task_descriptor *, Kern_Globals * );

void sys_exit(Task_descriptor *, Kern_Globals * );
