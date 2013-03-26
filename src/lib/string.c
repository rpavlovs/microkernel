#include "commonspace.h"

void strcpy( const char * source, char * target ) {
	int pos = -1;
	while( source[++pos] != '\0' ) {
		target[pos] = source[pos];
	}
	target[pos] = '\0';
}

int strcmp( const char *s1, const char *s2 ) {
	while( (*s1 && *s2) && ( *s1 == *s2 ) ){
		// This increment is not done in the while's condition because of short circuiting.
		*s1++; 
		*s2++;
	}
	return *s1 - *s2; 
}


char *substr( char *dest, const char *src, int initial_pos, int size ){
	char *ret_val = dest;

	// Move the src string to the position where it should start copying. 
	while( initial_pos-- )
		src++;

	// Copy the contents
	do
	{
		if ( !size-- )
			return ret_val; 
	} while( ( *dest++ = *src++ ) ); 

	// Fill the extra space. 
	while ( size-- )
        *dest++ = 0;

	return ret_val; 
}

int strlen( const char *str ){
	const char *ptr = str; 
	while( *str )
		++str; 
	return str - ptr; 
}

int append_char( char *str, char c, int num ){
	char *ptr = str; 
	while( num-- ){
		*str++ = c; 
	}
	*str = '\0'; 
	return str - ptr; 
}
