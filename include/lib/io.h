#ifndef __BWIO_H__
#define __BWIO_H__

#define DBG_FATAL	 	0	// panic and assert only
#define DBG_USR		 	1	// user tasks 
#define DBG_SYS		 	2	// sytem/user tasks
#define DBG_REQ		 	3	// kernel requests + sytem/user tasks
#define DBG_KERN 		4	// all

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

#define COM1	0
#define COM2	1

#define ON	1
#define	OFF	0

int bwsetfifo( int channel, int state );

int bwsetspeed( int channel, int speed );

int bwputc( int channel, char c );

int bwgetc( int channel );

int bwputx( int channel, char c );

int bwputstr( int channel, char *str );

int bwputr( int channel, unsigned int reg );

void bwputw( int channel, int n, char fc, char *bf );

void bwprintf( int channel, char *format, ... );

void debug( int threshhold_lvl, char *message, ... );

void assert( int condition, char * message, ... );

void panic( char * message, ... );


void todo_debug( int value, int channel ); 

#endif