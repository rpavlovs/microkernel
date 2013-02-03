#include "commonspace.h"

void assert( int statement, char * err_msg ) {
	if( !statement ) bwprintf( COM2, "Assertion failed: %s\n\r", err_msg );
}

void panic( char * panic_reason ) {
	bwprintf( COM2, "Kernel panic: %s\n\r", panic_reason );
	for( ; ; );
}

void my_strcpy( const char * source, char * target ) {
	int pos = -1;
	while( source[++pos] != '\0' ) {
		target[pos] = source[pos];
	}
	target[pos] = '\0';
}

void mem_cpy( const char * source, char * target, const unsigned int len ) {
	int pos = -1;
	while( ++pos < len ) {
		target[pos] = source[pos];
	}
}

int strcmp( const char *s1, const char *s2 ) {
    while( (*s1 && *s2) && (*s1++ == *s2++) );
    return *(--s1) - *(--s2);
}

