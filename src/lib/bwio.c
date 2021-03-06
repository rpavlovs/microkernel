/*
 * bwio.c - busy-wait I/O routines for diagnosis
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */
#include "commonspace.h"

/*
 * The UARTs are initialized by RedBoot to the following state
 * 	115,200 bps
 * 	8 bits
 * 	no parity
 * 	fifos enabled
 */
int bwsetenabled( int channel, int state ) {
	int *ctrl, buf;
	switch( channel ) {
	case COM1:
		ctrl = (int *)( UART1_BASE + UART_CTLR_OFFSET );
    	break;
	case COM2:
    	ctrl = (int *)( UART2_BASE + UART_CTLR_OFFSET );
    	break;
    case COM3:
    	ctrl = (int *)( UART3_BASE + UART_CTLR_OFFSET );
    	break;
	default:
    	return -1;
    	break;
	}
	buf = *ctrl;
	buf = state ? buf | UARTEN_MASK : buf & ~UARTEN_MASK;
	*ctrl = buf;
	return 0;
}

int bwsetfifo( int channel, int state ) {
	int *line, buf;
	switch( channel ) {
	case COM1:
		line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
    	break;
	case COM2:
    	line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
    	break;
    case COM3:
    	line = (int *)( UART3_BASE + UART_LCRH_OFFSET );
    	break;
	default:
    	return -1;
    	break;
	}
	buf = *line;
	buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
	*line = buf;
	return 0;
}

int bwsetspeed( int channel, int speed ) {
	int *mid, *low;
	switch( channel ) {
	case COM1:
		mid = (int *)( UART1_BASE + UART_LCRM_OFFSET );
		low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
	    break;
	case COM2:
		mid = (int *)( UART2_BASE + UART_LCRM_OFFSET );
		low = (int *)( UART2_BASE + UART_LCRL_OFFSET );
	    break;
	case COM3:
		mid = (int *)( UART3_BASE + UART_LCRM_OFFSET );
		low = (int *)( UART3_BASE + UART_LCRL_OFFSET );
	    break;
	default:
	    return -1;
	    break;
	}
	switch( speed ) {
	case 115200:
		*mid = 0x0;
		*low = 0x3;
		return 0;
	case 2400:
		*low = 0xbf;
		*mid = 0x0;
		return 0;
	default:
		return -1;
	}
}

int bwputc( int channel, char c ) {
	int *flags, *data;
	switch( channel ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	case COM3:
		flags = (int *)( UART3_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART3_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}
	while( ( *flags & TXFF_MASK ) ) ;
	*data = c;
	return 0;
}

char bwc2x( char ch ) {
	if ( (ch <= 9) ) return '0' + ch;
	return 'a' + ch - 10;
}

int bwputx( int channel, char c ) {
	char chh, chl;

	chh = bwc2x( c / 16 );
	chl = bwc2x( c % 16 );
	bwputc( channel, chh );
	return bwputc( channel, chl );
}

int bwputr( int channel, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) bwputx( channel, ch[byte] );
	return bwputc( channel, ' ' );
}

int bwputstr( int channel, char *str ) {
	while( *str ) {
		if( bwputc( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void bwputw( int channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) bwputc( channel, fc );
	while( ( ch = *bf++ ) ) bwputc( channel, ch );
}

int bwgetc( int channel ) {
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
	case COM3:
		flags = (int *)( UART3_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART3_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}
	while ( !( *flags & RXFF_MASK ) ) ;
	c = *data;
	return c;
}

int bwa2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

char bwa2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = bwa2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void bwui2a( unsigned int num, unsigned int base, char *bf ) {
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

void bwi2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	bwui2a( num, 10, bf );
}

void bwformat ( int channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			bwputc( channel, ch );
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
				ch = bwa2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				bwputc( channel, va_arg( va, char ) );
				break;
			case 's':
				bwputw( channel, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				bwui2a( va_arg( va, unsigned int ), 10, bf );
				bwputw( channel, w, lz, bf );
				break;
			case 'd':
				bwi2a( va_arg( va, int ), bf );
				bwputw( channel, w, lz, bf );
				break;
			case 'x':
				bwui2a( va_arg( va, unsigned int ), 16, bf );
				bwputw( channel, w, lz, bf );
				break;
			case '%':
				bwputc( channel, ch );
				break;
			}
		}
	}
}

void bwprintf( int channel, char *fmt, ... ) {
    va_list va;

    va_start(va,fmt);
    bwformat( channel, fmt, va );
    va_end(va);
}

void bwdebug( int threshhold_dbg_lvl, int debug_area, char *fmt, ... ) {
  if( ( threshhold_dbg_lvl > DEBUG_LEVEL ) || ( ( DEBUG_AREAS ) & ( debug_area ) ) == 0 ) return;

  va_list va;

	switch( threshhold_dbg_lvl ) {
	case 0:
	case 1:
		bwprintf( COM2, " DEBUG: [U] ", threshhold_dbg_lvl );
		break;
	case 2:
		bwprintf( COM2, " DEBUG: [S] ", threshhold_dbg_lvl );
		break;
	case 3:
		bwprintf( COM2, " DEBUG: [R]  ", threshhold_dbg_lvl );
		break;
	case 4:
		bwprintf( COM2, " DEBUG: [K]   ", threshhold_dbg_lvl );
		break;
	default:
		bwprintf( COM2, " DEBUG: [?] ", threshhold_dbg_lvl );
	}

  va_start(va,fmt);
  bwformat( COM2, fmt, va );
  va_end(va);
  
  bwprintf( COM2, "\n" );
}

void bwassert( int statement, char *fmt, ... ) {
	if( statement ) return;
	va_list va;

	bwprintf( COM2, "ASSERT: " );

    va_start(va,fmt);
    bwformat( COM2, fmt, va );
    va_end(va);

	bwprintf( COM2, "\n" );

	Shutdown();
	for( ; ; );
}


void bwpanic( char *fmt, ... ) {
	va_list va;
	bwprintf( COM2, " PANIC: " );

    va_start(va,fmt);
    bwformat( COM2, fmt, va );
    va_end(va);

	bwprintf( COM2, "\n" );

	Shutdown();
	for( ; ; );
}
