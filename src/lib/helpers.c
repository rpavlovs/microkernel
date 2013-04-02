#include "userspace.h"

// ----------------------------------------------------------------------
// Miscellaneous
// ----------------------------------------------------------------------

int is_train_id( int id ) {
	int TRAIN_IDS[NUM_TRAINS_SUPPORTED] = LIST_TRAINS_SUPPORTED;
	int i;
	for( i = 0; i < NUM_TRAINS_SUPPORTED; ++i ) {
		if( TRAIN_IDS[i] == id ) return 1;
	}
	return 0;
}

// ----------------------------------------------------------------------
// CLI Helpers
// ----------------------------------------------------------------------
int cursorPositioning( char *str_buff, int row, int column ){
	int size = 0; 
	size += sprintf( ( str_buff + size ), CURSOR_POS_STR, row, column ); 
	return size; 
}

int clearScreen( char *str_buff ){
	int size;
	size = sprintf( str_buff, CLEAR_SCREEN_STR ); 
	size += sprintf( ( str_buff + size ), MOVE_CURSOR_UP_LEFT ); 
	return size;
}

int saveCursor( char *str_buff ){
	return sprintf( str_buff, CURSOR_SAVE ); 
}

int hideCursor( char *str_buff ){
	return sprintf( str_buff, CURSOR_HIDE_STR ); 
}

int showCursor( char *str_buff ){
	return sprintf( str_buff, CURSOR_SHOW_STR ); 
}

int restoreCursor( char *str_buff ){
	return sprintf( str_buff, CURSOR_RESTORE ); 
}

int horizontal_line( char *str_buff, int length ) {
	char *ptr = str_buff;

	while( length --> 0 ) ptr += sprintf( ptr, "#" );

	return (ptr - str_buff);	
}

int vertical_line( char *str_buff, int length ) {
	char *ptr = str_buff;

	while( length --> 0 ) ptr += sprintf( ptr, "##\033[B\033[2D" );

	return (ptr - str_buff);	
}
