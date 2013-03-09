#include "userspace.h"

#define CPU_UTIL_TASK_DELAY		25
#define CPU_UTIL_ROW_POS		4
#define CPU_UTIL_COL_POS		18
#define CMD_ROW_POS				20
#define CMD_COL_POS				10

void print_cpu_util( char *str_buff, int current_time_ticks ){
	int cpu_util; 
	
	char *ptr = str_buff; 
	ptr += saveCursor( ptr ); 
	ptr += hideCursor( ptr );
	ptr += cursorPositioning( ptr, CPU_UTIL_ROW_POS, CPU_UTIL_COL_POS );
	ptr += sprintf( ptr, "%0d ", cpu_util ); 
	ptr += restoreCursor( ptr ); 
	
	// Print cpu_util
	Putstr( COM2, str_buff );
}

void draw_cpu_util() {
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
		delay = CPU_UTIL_TASK_DELAY - ( current_time_ticks % 10 );
		Delay( delay ); 
	}
}
