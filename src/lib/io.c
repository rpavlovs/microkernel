/*
 * bwio.c - busy-wait I/O routines for diagnosis
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#include <config/ts7200.h>
#include <config/system.h>
#include <lib/io.h>
#include <kernel/syscall.h>

char c2x( char ch ) {
	if ( (ch <= 9) ) return '0' + ch;
	return 'a' + ch - 10;
}

int putx( int channel, char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	Putc( channel, chh );
	return Putc( channel, chl );
}

int putr( int channel, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) putx( channel, ch[byte] );
	return Putc( channel, ' ' );
}

int putstr( int channel, char *str ) {
	while( *str ) {
		if( Putc( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void putw( int channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) Putc( channel, fc );
	while( ( ch = *bf++ ) ) Putc( channel, ch );
}

int getc( int channel ) {
	int *flags, *data;
	unsigned char c;

	switch( channel ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}
	while ( !( *flags & RXFF_MASK ) ) ;
	c = *data;
	return c;
}

int a2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

char a2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = a2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void ui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

void i2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	ui2a( num, 10, bf );
}

void format ( int channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			Putc( channel, ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = a2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				Putc( channel, va_arg( va, char ) );
				break;
			case 's':
				putw( channel, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				putw( channel, w, lz, bf );
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				putw( channel, w, lz, bf );
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				putw( channel, w, lz, bf );
				break;
			case '%':
				Putc( channel, ch );
				break;
			}
		}
	}
}

void printf( int channel, char *fmt, ... ) {
    va_list va;

    va_start(va,fmt);
    format( channel, fmt, va );
    va_end(va);
}

void debug( int threshhold_dbg_lvl, char *fmt, ... ) {
    if( threshhold_dbg_lvl > DEBUG_LEVEL ) return;
    va_list va;

	switch( threshhold_dbg_lvl ) {
	case 0:
	case 1:
		printf( COM2, " DEBUG: [U] ", threshhold_dbg_lvl );
		break;
	case 2:
		printf( COM2, " DEBUG: [S] ", threshhold_dbg_lvl );
		break;
	case 3:
		printf( COM2, " DEBUG: [R]  ", threshhold_dbg_lvl );
		break;
	case 4:
		printf( COM2, " DEBUG: [K]   ", threshhold_dbg_lvl );
		break;
	default:
		printf( COM2, " DEBUG: [?] ", threshhold_dbg_lvl );
	}

    va_start(va,fmt);
    format( COM2, fmt, va );
    va_end(va);
    
    printf( COM2, "\n" );
}

void assert( int statement, char *fmt, ... ) {
	
	if( statement ) return;
	va_list va;

	printf( COM2, "ASSERT: " );

    va_start(va,fmt);
    format( COM2, fmt, va );
    va_end(va);

	printf( COM2, "\n" );

	for( ; ; );
}

void panic( char *fmt, ... ) {
	va_list va;
	printf( COM2, " PANIC: " );

    va_start(va,fmt);
    format( COM2, fmt, va );
    va_end(va);

	printf( COM2, "\n" );

	for( ; ; );
}

void todo_debug( int value, int channel ) {
	int *ptr; 
	if ( channel == 0 ) {
		ptr = ( int * ) ( 0x01dffe50 );
	} else if( channel == 1 ) {
		ptr = ( int * ) ( 0x01dffe54 );
	} else {
		ptr = ( int * ) ( 0x01dffe58 );
	}
	*ptr = value; 
}