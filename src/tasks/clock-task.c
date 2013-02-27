#include "userspace.h"

#define CLOCK_TASK_DELAY		10
#define CLOCK_ROW_POS			4
#define CLOCK_COL_POS			18
#define CMD_ROW_POS			20
#define CMD_COL_POS			10

#define CURSOR_POS_STR			"\033[%d;%dH"
#define CURSOR_HIDE_STR			"\033[?25l"
#define CURSOR_SHOW_STR		"\033[?25h"
#define CLEAR_SCREEN_STR		"\033[2J"
#define MOVE_CURSOR_UP_LEFT		"\033[H"

//#define CURSOR_POS_STR			"03[%d;%dH\n"
//#define CURSOR_HIDE_STR			"03[?25l\n"
//#define CURSOR_SHOW_STR		"03[?25h\n"
//#define CLEAR_SCREEN_STR		"03[2J\n"
//#define MOVE_CURSOR_UP_LEFT		"03[H\n"

// TODO: Put this in a helper functions file. 
/*
int cursorPositioning( char *str_buff, int row, int column ){
	return sprintf( str_buff, CURSOR_POS_STR, row, column ); 
}

int clearScreen( char *str_buff ){
	int size;
	size = sprintf( str_buff, CLEAR_SCREEN_STR ); 
	size += sprintf( ( str_buff + size ), MOVE_CURSOR_UP_LEFT ); 
	return size;
}

int hideCursor( char *str_buff ){
	return sprintf( str_buff, CURSOR_HIDE_STR ); 
}

int showCursor( char *str_buff ){
	return sprintf( str_buff, CURSOR_SHOW_STR ); 
}
*/
void cursorPositioning( int row, int column ){
	bwprintf( COM2, "\033[%d;%dH", row, column ); 
}

void clearScreen(){
	bwprintf( COM2, CLEAR_SCREEN_STR ); 
	bwprintf( COM2, MOVE_CURSOR_UP_LEFT ); 
}

void hideCursor(){
	bwprintf( COM2, CURSOR_HIDE_STR ); 
}

void showCursor(){
	bwprintf( COM2, CURSOR_SHOW_STR );
}

void print_time( char *str_buff, int current_time_ticks ){
	// Get the time in terms of minutes, seconds and tenths of seconds. 
	int minutes, seconds, tenths, current_time; 
	/*current_time = current_time_ticks / 10; // Set the scale to tenths of seconds. 
	tenths = ( current_time % 10 ); 
	current_time -= tenths; 
	current_time /= 10; 
	seconds = ( current_time % 60 ); 
	minutes = ( current_time / 6000 );
	 */
	
	current_time = current_time_ticks / 10; // Set the scale to tenths of seconds. 
	tenths = ( ( current_time % 60 ) % 10 ); 
	current_time /= 10; 
	
	minutes = (current_time) / 60;
	seconds = ( current_time % 60 );
	//tenths = ( ( current_time %60 ) % 10 );
	
	
	//minutes = (current_time) / 6000;
	//seconds = (current_time % 60) / 100;
	//tenths = ((current_time  % 60) % 100) / 10;
	
	
	
	// Print time in right position and return to original cmd position.
	// Put the clock and all the formatting information in a buffer. 
	char *temp_buffer = str_buff; 
	//temp_buffer += hideCursor( temp_buffer );
	//temp_buffer += cursorPositioning( temp_buffer, CLOCK_ROW_POS, CLOCK_COL_POS );
	temp_buffer += sprintf( temp_buffer, "%d:%d:%d\n", minutes, seconds, tenths ); 
	//temp_buffer += sprintf( temp_buffer, "%d\n", current_time_ticks	 ); 
	//temp_buffer += cursorPositioning( temp_buffer, CMD_ROW_POS, CMD_COL_POS );
	//temp_buffer += showCursor( temp_buffer );
	
	// Print clock
	//putstr( COM2, str_buff );
	Putstr( COM2, str_buff );
	//bwprintf( COM2, str_buff ); 
	
	
//	hideCursor();
//	cursorPositioning( CLOCK_ROW_POS, CLOCK_COL_POS );
//	bwprintf( COM2, "%d:%d:%d", minutes, seconds, tenths ); 
//	cursorPositioning( CMD_ROW_POS, CMD_COL_POS );
//	showCursor();	
}

void draw_clock(){
	// Initialization
	char buff[ 300 ];
	char *str_buff = buff; 
	
	int delay; 
	
	// TODO: This should be in a separate file. 
	FOREVER {
		// Gets the current time in ticks (a tick is 10 milliseconds). 
		int current_time_ticks = Time(); 
		
		// This will re-draw the clock in the screen. 
		print_time( str_buff, current_time_ticks );
		
		// Calculate the clock delay. 
		// The idea is that the clock should be awakened every 100 ms, even if there were delays.
		delay = CLOCK_TASK_DELAY - ( current_time_ticks % 10 );
		Delay( delay ); 
	}
}
