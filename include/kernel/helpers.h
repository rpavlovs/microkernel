#ifndef __HELPERS__
#define __HELPERS__

void assert( int, char *);

void panic( char *);

void debug( char *);

void str_cpy( const char * , char * );

void mem_cpy( const char * , char * , const unsigned int );

int strcmp(const char *s1, const char *s2);

#endif
