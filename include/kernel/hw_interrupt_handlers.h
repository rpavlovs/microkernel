#ifndef HW_INTERRUPT_HANDLERS_H
#define	HW_INTERRUPT_HANDLERS_H

#include <kernelspace.h>

// HW Interrupts Types
// -> VIC1
#define TIMER1_INT				0x10			
#define TIMER2_INT				0x20			

// -> VIC2
#define TIMER3_INT				0x00080000		
#define UART1_INT				0x00100000		
#define UART2_INT				0x00400000		

// UART Interrupt Flags
// -- UART interrupts are ORed in a common interrupt. These are the flags
//    used to determine which ones are enabled inside the "uber" interrupt.
#define UART_MODEM_INT_STATUS	0x1		// Modem interrupt status			(UARTMSINTR).
#define UART_RX_INT_STATUS		0x2		// Receive interrupt status			(UARTRXINTR).
#define UART_TX_INT_STATUS		0x4		// Transmit interrupt status		(UARTTXINTR).
#define UART_RX_TIME_INT_STATUS 0x8		// Receive timeout interrupt status (UARTRTINTR)

void timer_hwi_handler( Kern_Globals *GLOBALS );

void uart1_hwi_handler( Kern_Globals *GLOBALS );

void uart2_hwi_handler( Kern_Globals *GLOBALS );

#endif	/* HW_INTERRUPT_HANDLERS_H */

