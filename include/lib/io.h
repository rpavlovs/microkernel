#ifndef __IO_H__
#define __IO_H__

#define DBG_FATAL	 	0	// panic and assert only
#define DBG_USR		 	1	// user tasks 
#define DBG_SYS		 	2	// sytem/user tasks
#define DBG_REQ		 	3	// kernel requests + sytem/user tasks
#define DBG_KERN 		4	// all

#define COM1	0
#define COM2	1

#define ON	1
#define	OFF	0

int putc( int channel, char c );

int getc( int channel );

int putx( int channel, char c );

int putstr( int channel, char *str );

int putr( int channel, unsigned int reg );

void putw( int channel, int n, char fc, char *bf );

void printf( int channel, char *format, ... );

void debug( int threshhold_lvl, char *message, ... );

void assert( int condition, char * message, ... );

void panic( char * message, ... );


void todo_debug( int value, int channel ); 

#endif
