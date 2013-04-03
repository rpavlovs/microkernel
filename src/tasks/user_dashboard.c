#include "userspace.h"

void draw_clock( char *str_buff, int current_time_ticks );
void draw_cpu_util( char *str_buff, int cpu_util );

void draw_screen_statics();
void draw_sysinfo_statics();

void user_dashboard() {
	char buff[ 300 ];
	char *str_buff = buff;
	System_data sysdata;
	int curr_time, last_util_update = 0;

	draw_screen_statics();
	draw_sysinfo_statics();
	
	Delay( 50 );

	FOREVER {
		curr_time = Time();
		
		// Update clock
		//draw_clock( str_buff, curr_time );

		// Update CPU utilization
		if( last_util_update + CPU_UTIL_UPDATE_INTERVAL < curr_time ) {
			last_util_update = curr_time;
			GetSysdata( &sysdata );
			//draw_cpu_util( str_buff, sysdata.cpu_utilization );
		}

		DelayUntil( curr_time + CLOCK_UPDATE_INTERVAL );
	}
}

void draw_clock( char *str_buff, int current_time_ticks ){
	int minutes, seconds, tenths, current_time; 
	char *ptr = str_buff; 
	
	current_time = current_time_ticks / 10; // Set the scale to tenths of seconds. 
	tenths = ( ( current_time % 60 ) % 10 ); 
	current_time /= 10; 
	minutes = (current_time) / 60;
	seconds = ( current_time % 60 );
	
	ptr += hideCursor( ptr );
	ptr += saveCursor( ptr ); 
	ptr += cursorPositioning( ptr, SYSINFO_POS_ROW, SYSINFO_POS_COL + 15 );
	ptr += sprintf( ptr, "%d:%d.%d ", minutes, seconds, tenths ); 
	ptr += restoreCursor( ptr );
	ptr += showCursor( ptr );
	
	Putstr( COM2, str_buff );
}

void draw_cpu_util( char *str_buff, int cpu_util ){
	
	char *ptr = str_buff;
	ptr += hideCursor( ptr );
	ptr += saveCursor( ptr ); 
	ptr += cursorPositioning( ptr, SYSINFO_POS_ROW+1, SYSINFO_POS_COL + 15 );
	ptr += sprintf( ptr, "%d ", cpu_util ); 
	ptr += restoreCursor( ptr );
	
	// Print cpu_util
	Putstr( COM2, str_buff );
}

void draw_screen_statics() {
	char buff[4000];
	char *ptr = buff;
	
	ptr += hideCursor( ptr );
	ptr += clearScreen( ptr );
	ptr += horizontal_line( ptr, SCREEN_WIDTH);
	
	ptr += cursorPositioning( ptr, 1, SCREEN_WIDTH - 1);
	ptr += vertical_line( ptr, SCREEN_HEIGHT);
	
	ptr += cursorPositioning( ptr, SCREEN_HEIGHT, 1);
	ptr += horizontal_line( ptr, SCREEN_WIDTH);

	ptr += cursorPositioning( ptr, 1, 1);
	ptr += vertical_line( ptr, SCREEN_HEIGHT);
	
	ptr += cursorPositioning( ptr, 2, SCREEN_WIDTH - TRACK_WIDTH - 2 );
	ptr += vertical_line( ptr, TRACK_HEIGHT);

	ptr += cursorPositioning( ptr, TRACK_HEIGHT + 2, 1 );
	ptr += horizontal_line( ptr, SCREEN_WIDTH );

	ptr += cursorPositioning( ptr, TRACK_HEIGHT + TRAINS_POS_HEIGHT + 3, 1 );
	ptr += horizontal_line( ptr, SCREEN_WIDTH );

	ptr += cursorPositioning( ptr, 3 , 3 );
	ptr += horizontal_line( ptr, SCREEN_WIDTH - TRACK_WIDTH - 5 );

	ptr += cursorPositioning( ptr, SYSINFO_HEIGHT + 4, 3 );
	ptr += horizontal_line( ptr, SCREEN_WIDTH - TRACK_WIDTH - 5 );

	ptr += cursorPositioning( ptr, SYSINFO_HEIGHT + REC_SENSORS_HEIGHT + 5, 3 );
	ptr += horizontal_line( ptr, SCREEN_WIDTH - TRACK_WIDTH - 5 );

	ptr += cursorPositioning( ptr,
		SYSINFO_HEIGHT + REC_SENSORS_HEIGHT + SWITCH_POS_HEIGHT +6, 3 );
	ptr += horizontal_line( ptr, SCREEN_WIDTH - TRACK_WIDTH - 5 );

	ptr += cursorPositioning( ptr, 2, 4 );
	ptr += sprintf( ptr, "Train control ##   Track A" );

	ptr += cursorPositioning( ptr, 8, 1 );
	ptr += sprintf( ptr, 
		"##       Recent Sensors\n\n\n\n"
		"##       Switch Positions      \n"
		"##  1:  4:  7:  10:  13:  16:  \n"
		"##  2:  5:  8:  11:  14:  17:  \n"
		"##  3:  6:  9:  12:  15:  18:  \n"
		"##  153:   154:   155:   156:  \n");

	ptr += cursorPositioning( ptr, 33, 4 );
	ptr += sprintf( ptr, 
		"Trains Position\n"
		"## -Train 1: Id ->      Landmark ->        Offset ->\n"
		"## -Train 2: Id ->      Landmark ->        Offset ->\n"
		"## -Train 3: Id ->      Landmark ->        Offset ->\n");

	ptr += cursorPositioning( ptr, CLI_CURSOR_POS );
	ptr += showCursor( ptr );

	Putstr( COM2, buff );
}

void draw_sysinfo_statics() {
	char buff[200];
	char *ptr = buff;

	ptr += hideCursor( ptr );
	ptr += saveCursor( ptr ); 
	ptr += cursorPositioning( ptr, SYSINFO_POS_ROW, SYSINFO_POS_COL+1 );
	ptr += sprintf( ptr, "Uptime" );
	ptr += cursorPositioning( ptr, SYSINFO_POS_ROW+1, SYSINFO_POS_COL+1 );
	ptr += sprintf( ptr, "CPU Load" );
	ptr += cursorPositioning( ptr, SYSINFO_POS_ROW+2, SYSINFO_POS_COL+1 );
	ptr += sprintf( ptr, "Tasks Alive" );
	ptr += restoreCursor( ptr );
	ptr += showCursor( ptr );

	Putstr( COM2, buff );
}
