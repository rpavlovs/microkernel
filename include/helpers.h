#ifndef __HELPERS_H__
#define __HELPERS_H__

//
// Helper defines
// 

#define FOREVER		for( ; ; )

//
// Helper funtions
//

void assert( int, char * );

void panic( char * );

void debug( char * );

void my_strcpy( const char * , char * );

void mem_cpy( const char * , char * , const unsigned int );

int strcmp( const char *s1, const char *s2 );

#endif
