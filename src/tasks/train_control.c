#include <userspace.h>

void setupScreen() {
	char buff[4000];
	char *ptr = buff;

	ptr += sprintf( ptr, "\033[2J\033[;H" );
	ptr += sprintf( ptr,
		"################################################################################\n"
		"## Train Control #   Track A    ## ____12__11_____________________________    ##\n"
		"##################################     /   /                              \\   ##\n"
		"## Uptime        mm:ss          ## ___4  14__________13______10___________ \\  ##\n"
		"## CPU Load      dd             ##   /   /            \\      /            \\ | ##\n"
		"## Active Tasks  dd             ## _/   /              9C | 9B             \\| ##\n"
		"##################################     |                 \\|/                9 ##\n"
		"##       Recent Sensors         ##     |                  |                 | ##\n"
		"##                              ##     |                 /|\\                8 ##\n"
		"##                              ## __   \\              99 | 9A             /| ##\n"
		"##################################   \\   \\             /     \\            / | ##\n"
		"##       Switch Positions       ## ___1   15__________16______17_________/ /  ##\n"
		"##  1:  4:  7:  10:  13:  16:   ##     \\   \\                              /   ##\n"
		"##  2:  5:  8:  11:  14:  17:   ## _____2   \\_______6___________7________/    ##\n"
		"##  3:  6:  9:  12:  15:  18:   ##       \\           \\         /              ##\n"
		"##  153:   154:   155:   156:   ## _______3__________18_______5______________ ##\n"
		"################################################################################\n"
		"## Executed Command:\n"
		"##\n"
		"##\n"
		"##\n"
		"##\n"
		"## ~>\n"
		"################################################################################\n");
	ptr += sprintf( ptr, "\033[%d;7H", UI_CLI_CMD_LINE_ROW );
	// printf( COM2, "%s", buff );
	Putstr( COM2, buff );
}

void train_control() {
	
	setupScreen();
	Create( SENSOR_SERVER_PRIORITY, sensors_server );
	Create( SWITCHES_SERVER_PRIORITY, switchserver );
	Create( CLI_PRIORITY, task_cli );
	Create( CLOCK_TASK_PRIORITY, draw_clock );

	Exit();
}

