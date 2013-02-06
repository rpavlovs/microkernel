#include "commonspace.h"


void strcpy( const char * source, char * target ) {
	int pos = -1;
	while( source[++pos] != '\0' ) {
		target[pos] = source[pos];
	}
	target[pos] = '\0';
}

int strcmp( const char *s1, const char *s2 ) {
    while( (*s1 && *s2) && (*s1++ == *s2++) );
    return *(--s1) - *(--s2);
}

