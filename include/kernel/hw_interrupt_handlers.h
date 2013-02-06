#ifndef HW_INTERRUPT_HANDLERS_H
#define	HW_INTERRUPT_HANDLERS_H

// HW Interrupts Types
#define TIMER1_INT				0x10
#define TIMER2_INT				0x20

void timer_hwi_handler( Kern_Globals *GLOBALS );

#endif	/* HW_INTERRUPT_HANDLERS_H */

