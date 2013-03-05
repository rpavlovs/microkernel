#include "userspace.h"

#define CLOCK_TASK_DELAY		10
#define CLOCK_ROW_POS			4
#define CLOCK_COL_POS			18
#define CMD_ROW_POS				20
#define CMD_COL_POS				10

#define CURSOR_SAVE				"\033[s"
#define CURSOR_RESTORE			"\033[u"
#define CURSOR_POS_STR			"\033[%d;%dH"
#define CURSOR_HIDE_STR			"\033[?25l"
#define CURSOR_SHOW_STR			"\033[?25h"
#define CLEAR_SCREEN_STR		"\033[2J"
#define MOVE_CURSOR_UP_LEFT		"\033[H"

// TODO: Put this in a helper functions file. 
int cursorPositioning( char *str_buff, int row, int column ){
	int size = 0; 
	size += sprintf( str_buff, CURSOR_SAVE ); 
	size += sprintf( ( str_buff + size ), CURSOR_POS_STR, row, column ); 
	return size; 
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

int restoreCursor( char *str_buff ){
	return sprintf( str_buff, CURSOR_RESTORE ); 
}

void print_time( char *str_buff, int current_time_ticks ){
	// Get the time in terms of minutes, seconds and tenths of seconds. 
	int minutes, seconds, tenths, current_time; 
	
	current_time = current_time_ticks / 10; // Set the scale to tenths of seconds. 
	tenths = ( ( current_time % 60 ) % 10 ); 
	current_time /= 10; 
	minutes = (current_time) / 60;
	seconds = ( current_time % 60 );
	
	
	// Print time in right position and return to original cmd position.
	// Put the clock and all the formatting information in a buffer. 
	char *temp_buffer = str_buff; 
	temp_buffer += hideCursor( temp_buffer );
	temp_buffer += cursorPositioning( temp_buffer, CLOCK_ROW_POS, CLOCK_COL_POS );
	temp_buffer += sprintf( temp_buffer, "%02d:%02d:%02d ", minutes, seconds, tenths ); 
	//temp_buffer += cursorPositioning( temp_buffer, CMD_ROW_POS, CMD_COL_POS );
	temp_buffer += restoreCursor( temp_buffer ); 
	//temp_buffer += showCursor( temp_buffer );
	
	// Print clock
	Putstr( COM2, str_buff );
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
