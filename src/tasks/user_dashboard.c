#include "userspace.h"

#define CLOCK_UPDATE_INTERVAL		10
#define CLOCK_ROW_POS				4
#define CLOCK_COL_POS				18

#define CPU_UTIL_UPDATE_INTERVAL	2
#define CPU_UTIL_ROW_POS			5
#define CPU_UTIL_COL_POS			18

#define CMD_ROW_POS					20
#define CMD_COL_POS					10

void draw_screen() {
	char buff[4000];
	char *ptr = buff;

	ptr += sprintf( ptr, "\033[2J\033[;H" );
	ptr += sprintf( ptr,
		"################################################################################\n"
		"## Train Control ##   Track A   ## ____12__11_____________________________    ##\n"
		"##################################     /   /_________13_______10_________ \\   ##\n"
		"## Uptime                       ## ___4   14          \\       /          \\ \\  ##\n"
		"## CPU Util                     ##   /   /             \\     /            \\ | ##\n"
		"## Active Tasks  dd             ## _/   /              9C | 9B             \\| ##\n"
		"##################################     |                 \\|/                9 ##\n"
		"##       Recent Sensors         ##     |                  |                 | ##\n"
		"##                              ## _   |                 /|\\                8 ##\n"
		"##                              ##  \\   \\              99 | 9A             /| ##\n"
		"################################## __1   \\             /     \\            / | ##\n"
		"##       Switch Positions       ##    \\   15_________16_______17_________/ /  ##\n"
		"##  1:  4:  7:  10:  13:  16:   ## ____2   \\_______6___________7__________/   ##\n"
		"##  2:  5:  8:  11:  14:  17:   ##      \\           \\         /               ##\n"
		"##  3:  6:  9:  12:  15:  18:   ## ______3__________18_______5_______________ ##\n"
		"##  153:   154:   155:   156:   ##                                            ##\n"
		"################################################################################\n"
		"## Trains Position                                                            ##\n"
		"## -Train 1: Id ->      Landmark ->        Offset ->                          ##\n"
		"## -Train 2: Id ->      Landmark ->        Offset ->                          ##\n"
		"## -Train 3: Id ->      Landmark ->        Offset ->                          ##\n"
		"################################################################################\n"
		"## Executed Command:\n"
		"##\n"
		"##\n"
		"##\n"
		"##\n"
		"## ~>\n"
		"################################################################################\n");
	ptr += sprintf( ptr, "\033[%d;7H", UI_CLI_CMD_LINE_ROW );
	Putstr( COM2, buff );
}

void draw_clock( char *str_buff, int current_time_ticks ){
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
	temp_buffer += sprintf( temp_buffer, "%d:%d.%d ", minutes, seconds, tenths ); 
	temp_buffer += restoreCursor( temp_buffer );
	temp_buffer += showCursor( temp_buffer );
	
	// Print clock
	Putstr( COM2, str_buff );
}

void draw_cpu_util( char *str_buff, int cpu_util ){
	
	char *ptr = str_buff;
	ptr += hideCursor( ptr );
	ptr += saveCursor( ptr ); 
	ptr += cursorPositioning( ptr, CPU_UTIL_ROW_POS, CPU_UTIL_COL_POS );
	ptr += sprintf( ptr, "%d ", cpu_util ); 
	ptr += restoreCursor( ptr );
	
	// Print cpu_util
	Putstr( COM2, str_buff );
}

void user_dashboard() {
	char buff[ 300 ];
	char *str_buff = buff;
	System_data sysdata;
	int curr_time, last_util_update = 0;

	draw_screen();
	
	Delay( 50 );

	FOREVER {
		curr_time = Time();
		
		// Update clock
		draw_clock( str_buff, curr_time );

		// Update CPU utilization
		if( last_util_update + CPU_UTIL_UPDATE_INTERVAL < curr_time ) {
			last_util_update = curr_time;
			GetSysdata( &sysdata );
			draw_cpu_util( str_buff, sysdata.cpu_utilization );
		}

		DelayUntil( curr_time + CLOCK_UPDATE_INTERVAL );
	}
}
