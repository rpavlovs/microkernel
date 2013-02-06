#ifndef __CONTEXT_SWITCH__
#define __CONTEXT_SWITCH__

#include "kernelspace.h"

// Constants
#define SVC_MODE_FLAG			0x13
#define IRQ_MODE_FLAG			0x1F
#define SYS_MODE_FLAG			0x12

#define INT_CONTROL_BASE_1		0x800B0000		// VIC 1
#define INT_CONTROL_BASE_2		0x800C0000		// VIC 2

#define IRQ_STATUS_OFFSET		0x0
#define FIQ_STATUS_OFFSET		0x4
#define INT_RAW_OFFSET			0x8
#define INT_SELECT_OFFSET		0xC
#define INT_ENABLE_OFFSET		0x10
#define INT_ENABLE_CLR_OFFSET	0x14
#define INT_SOFT_OFFSET			0x18
#define	INT_SOFT_CLEAR_OFFSET	0x1C
#define INT_VEC_ADDRESS_OFFSET	0x30

#define INT_RESET_VALUE			0x0


void ExecuteCSWIHandler( unsigned int, unsigned int, unsigned int );

int initialize_context_switching();

int install_interrupt_handler( unsigned int handlerLoc, unsigned int *vector );  

int execute_user_task(unsigned int, unsigned int, unsigned int);

void RetrieveSysCallArgs( int *, int, unsigned int);

void SetSysCallReturn( int, unsigned int);

void handle_request( int, Kern_Globals *);

void handle_swi( int , Kern_Globals * ); 

void handle_hwi( Kern_Globals * );

#endif
