#include "userspace.h"

#define CLOCK_TASK_DELAY	100
#define CLOCK_ROW_POS		10
#define CLOCK_COL_POS		10
#define CMD_ROW_POS			20
#define CMD_COL_POS			10

#define CURSOR_HIDE_STR		"\033[?25l"
#define CURSOR_SHOW_STR		"\033[?25h"
#define CLEAR_SCREEN_STR	"\033[2J"
#define MOVE_CURSOR_UP_LEFT	"\033[H"

// TODO: Put this in a helper functions file. 
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

/*
 void clock_redraw(struct print_buffer* pbuff, int minutes, int seconds, int tseconds){
	//Clear the screen
	ap_init_buff( pbuff );
	ap_putstr( pbuff, "\x1B[?25l");	//hide cursor
	ap_putstr( pbuff, "\x1B[1;51f");	//move to clock position
	ap_putstr( pbuff, "\x1B[1;32m");	//set attributes
	ap_printf( pbuff, "%d:%d:%d ", minutes, seconds, tseconds);
	ap_putstr( pbuff, "\x1B[0m");	//clear attributes
	ap_putstr( pbuff, "\x1B[u");	//return to CL 
	ap_putstr( pbuff, "\x1B[?25h");	//show cursor
	Putbuff( COM2, pbuff ); 
}

 
 */
void print_time( int current_time_ticks ){
	// Get the time in terms of minutes, seconds and tenths of seconds. 
	int minutes, seconds, tenths, current_time; 
	current_time = current_time_ticks / 100; // Set the scale to tenths of seconds. 
	tenths = ( current_time % 10 ); 
	current_time -= tenths; 
	
	seconds = ( current_time % 60 ); 
	minutes = ( current_time / 60 );
	
	// Print time in right position and return to original cmd position. 
	hideCursor();
	cursorPositioning( CLOCK_ROW_POS, CLOCK_COL_POS );
	bwprintf( COM2, "%d:%d:%d", minutes, seconds, tenths ); 
	cursorPositioning( CMD_ROW_POS, CMD_COL_POS );
	showCursor();	
}

void draw_clock(){
	
	// TODO: This should be in a separate file. 
	
	FOREVER {
		// Gets the current time in ticks (a tick is 10 milliseconds). 
		int current_time_ticks = Time(); 
		
		// This will re-draw the clock in the screen. 
		print_time( current_time_ticks );
		
		// Delay the clock for 100 ms. 
		DelayUntil( CLOCK_TASK_DELAY ); 
	}
}
