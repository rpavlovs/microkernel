#ifndef __HELPERS_H__
#define __HELPERS_H__

// ----------------------------------------------------------------------
// Debug Helpers and Macros
// ----------------------------------------------------------------------

// get the time pass since kernel init in ms
#define get_dbg_time() ( \
	( ((long)(*((int *)(Timer4ValueLow)))) + ((long)(*((char *)(Timer4ValueHigh)))) ) / 983 )

// ----------------------------------------------------------------------
// Miscellaneous
// ----------------------------------------------------------------------

// Checks if "id" is one of the supported train ids
int is_train_id( int num );

// -----------------------------------------------------------------------
// CLI Helpers
// -----------------------------------------------------------------------
#define CURSOR_SAVE				"\033[s"
#define CURSOR_RESTORE			"\033[u"
#define CURSOR_POS_STR			"\033[%d;%dH"
#define CURSOR_HIDE_STR			"\033[?25l"
#define CURSOR_SHOW_STR			"\033[?25h"
#define CLEAR_SCREEN_STR		"\033[2J"
#define MOVE_CURSOR_UP_LEFT		"\033[H"
#define MOVE_CURSOR_ONE_DOWN	"\033[B"


int saveCursor( char *str_buff ); 

int cursorPositioning( char *str_buff, int row, int column ); 

int clearScreen( char *str_buff ); 

int hideCursor( char *str_buff ); 

int showCursor( char *str_buff );

int restoreCursor( char *str_buff );

int horizontal_line( char *str_buff, int length );

int vertical_line( char *str_buff, int length );

#endif
