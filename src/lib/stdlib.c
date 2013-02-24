#include <lib/stdlib.h>

int atoi( char *p ) {
	int k = 0;
	while (*p) {
		k = (k<<3)+(k<<1)+(*p)-'0';
		p++;
	}
	return k;
}

void mem_cpy( const char * source, char * target, const unsigned int len ) {
	int pos = -1;
	while( ++pos < len ) {
		target[pos] = source[pos];
	}
}
