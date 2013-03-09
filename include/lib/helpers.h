#ifndef __HELPERS_H__
#define __HELPERS_H__

// Checks if "id" is one of the supported train ids
int is_train_id( int num );

// -----------------------------------------------------------------------------------------------------------------------------------------------
// CLI Helpers
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define CURSOR_SAVE				"\033[s"
#define CURSOR_RESTORE			"\033[u"
#define CURSOR_POS_STR			"\033[%d;%dH"
#define CURSOR_HIDE_STR			"\033[?25l"
#define CURSOR_SHOW_STR			"\033[?25h"
#define CLEAR_SCREEN_STR		"\033[2J"
#define MOVE_CURSOR_UP_LEFT		"\033[H"

int cursorPositioning( char *str_buff, int row, int column ); 

int clearScreen( char *str_buff ); 

int hideCursor( char *str_buff ); 

int showCursor( char *str_buff );

int restoreCursor( char *str_buff );

#endif
