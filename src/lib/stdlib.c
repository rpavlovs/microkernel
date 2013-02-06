#include <lib/stdlib.h>

void mem_cpy( const char * source, char * target, const unsigned int len ) {
	int pos = -1;
	while( ++pos < len ) {
		target[pos] = source[pos];
	}
}
