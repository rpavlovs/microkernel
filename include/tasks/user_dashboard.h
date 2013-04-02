#ifndef __DASHBOARD_H__
#define	__DASHBOARD_H__

// UI layouts

#define SCREEN_WIDTH 		159
#define SCREEN_HEIGHT 		60

#define SYSINFO_POS_ROW		4
#define SYSINFO_POS_COL		3
#define SYSINFO_HEIGHT	 	3

#define REC_SENSORS_HEIGHT	3

#define SWITCH_POS_HEIGHT	5

#define TRAINS_POS_HEIGHT 	4

// System info update frequencies

#define CLOCK_UPDATE_INTERVAL		10
#define CPU_UTIL_UPDATE_INTERVAL	2


void user_dashboard();


#endif	/* __DASHBOARD_H__ */
