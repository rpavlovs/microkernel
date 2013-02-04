#ifndef __CONTEXT_SWITCH__
#define __CONTEXT_SWITCH__

#include "kernelspace.h"

void swi_main_handler();

void ExecuteCSWIHandler( unsigned int, unsigned int, unsigned int );

int execute_user_task(unsigned int, unsigned int, unsigned int);

void RetrieveSysCallArgs( int *, int, unsigned int);

void SetSysCallReturn( int, unsigned int);

void handle_request( int, Kern_Globals *);

void initialize_context_switching();

#endif