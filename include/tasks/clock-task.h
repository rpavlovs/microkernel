#ifndef CLOCK_TASK_H
#define	CLOCK_TASK_H

// --------------------------------------------------------------------------------------
// Constants
// --------------------------------------------------------------------------------------
#define CLOCK_TASK_DELAY		10
#define CLOCK_ROW_POS			4
#define CLOCK_COL_POS			18
#define CMD_ROW_POS				20
#define CMD_COL_POS				10

// --------------------------------------------------------------------------------------
// Methods
// --------------------------------------------------------------------------------------
void draw_clock();

void print_time( char *str_buff, int current_time_ticks ); 

#endif	/* CLOCK_TASK_H */

