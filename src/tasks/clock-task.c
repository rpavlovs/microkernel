#include "userspace.h"

#define CLOCK_TASK_DELAY		10
#define CLOCK_ROW_POS			4
#define CLOCK_COL_POS			18
#define CMD_ROW_POS				20
#define CMD_COL_POS				10

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
	temp_buffer += saveCursor( temp_buffer ); 
	temp_buffer += hideCursor( temp_buffer );
	temp_buffer += cursorPositioning( temp_buffer, CLOCK_ROW_POS, CLOCK_COL_POS );
	temp_buffer += sprintf( temp_buffer, "%0d:%0d:%0d ", minutes, seconds, tenths ); 
	temp_buffer += restoreCursor( temp_buffer ); 
	
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
