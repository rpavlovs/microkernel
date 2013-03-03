#include "commonspace.h"

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

int sputw( char *buffer, int n, char fc, char *bf, int lj) {
	char ch;
	char *p = bf;
	char *buffer_pos = buffer;

	while( *p++ && n > 0 ) n--;
	if( lj ) {
		while( ( ch = *bf++ ) )  *(buffer_pos++) = ch;
		while( n-- > 0 ) *(buffer_pos++) = fc;
	} else {
		while( n-- > 0 ) *(buffer_pos++) = fc;
		while( ( ch = *bf++ ) )  *(buffer_pos++) = ch;
	}
	return buffer_pos - buffer;
}

int sformat ( char *buffer, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w, lj;
	char *buffer_pos = buffer;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			*(buffer_pos++) = ch;
		else {
			lz = 0; w = 0; lj = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '-':
				lj = 1; ch = *(fmt++);
				break;
			}
			switch ( ch ) {
			case 0:
				*buffer_pos = '\0';
				return (int) (buffer_pos - buffer);
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
			case 0:
				*buffer_pos = '\0';
				return (int) (buffer_pos - buffer);
			case 'c':
				*(buffer_pos++) = va_arg( va, char );
				break;
			case 's':
				buffer_pos += sputw( buffer_pos, w, 0, va_arg( va, char* ), lj );
				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				buffer_pos += sputw( buffer_pos, w, lz, bf, lj );
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				buffer_pos += sputw( buffer_pos, w, lz, bf, lj );
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				buffer_pos += sputw( buffer_pos, w, lz, bf, lj );
				break;
			case '%':
				*(buffer_pos++) = ch;
				break;
			}
		}
	}
	*buffer_pos = '\0';
	return (int) (buffer_pos - buffer);
}

int sprintf( char *buffer, char *fmt, ... ) {
	va_list va;
	int size;

    va_start(va,fmt);
    size = sformat( buffer, fmt, va );
    va_end(va);
    return size;
}

void debug( int threshhold_dbg_lvl, char *fmt, ... ) {
//   if( threshhold_dbg_lvl > DEBUG_LEVEL ) return;
//   va_list va;
//
//	switch( threshhold_dbg_lvl ) {
//	case 0:
//	case 1:
//		bwprintf( COM2, " DEBUG: [U] ", threshhold_dbg_lvl );
//		break;
//	case 2:
//		bwprintf( COM2, " DEBUG: [S] ", threshhold_dbg_lvl );
//		break;
//	case 3:
//		bwprintf( COM2, " DEBUG: [R]  ", threshhold_dbg_lvl );
//		break;
//	case 4:
//		bwprintf( COM2, " DEBUG: [K]   ", threshhold_dbg_lvl );
//		break;
//	default:
//		bwprintf( COM2, " DEBUG: [?] ", threshhold_dbg_lvl );
//	}
//
//   va_start(va,fmt);
//   bwformat( COM2, fmt, va );
//   va_end(va);
//   
//   bwprintf( COM2, "\n" );
}

void assert( int statement, char *fmt, ... ) {
	
//	if( statement ) return;
//	va_list va;
//
//	bwprintf( COM2, "ASSERT: " );
//
//    va_start(va,fmt);
//    bwformat( COM2, fmt, va );
//    va_end(va);
//
//	bwprintf( COM2, "\n" );
//
//	for( ; ; );
}

void panic( char *fmt, ... ) {
	
//	va_list va;
//	bwprintf( COM2, " PANIC: " );
//
//    va_start(va,fmt);
//	Create( 7, task_cli );
//    va_end(va);
//
//	bwprintf( COM2, "\n" );
//
//	for( ; ; );	
}

// dump -b 0x01dffe50 -l 4
void todo_debug( int value, int channel ) {
	int *ptr, addr;
	
	addr = 0x01dffe50;
	addr += channel * 4;
	ptr = ( int * ) addr;

	*ptr = value;
}















