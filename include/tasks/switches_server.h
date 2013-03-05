#ifndef SWITCHES_SERVER_H
#define	SWITCHES_SERVER_H

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define SWITCHES_SERVER_NAME	"switches_server"
#define SWITCHES_SERVER_PRIORITY	12
#define NUM_SWITCHES			22
#define LAST_LOW_SWITCH_INDEX	18
#define INIT_HIGH_SWITCH_INDEX	153
#define LAST_HIGH_SWITCH_INDEX     156

#define SWITCH_STRAIGHT_POS		'S'
#define SWITCH_CURVE_POS		'C'

static const int Switches_screen_pos[NUM_SWITCHES][2] = { 
	{ 13, 7	 },			// Switch 1
	{ 14, 7  },			// Switch 2
	{ 15, 7  },			// Switch 3
	{ 13, 11 },			// Switch 4
	{ 14, 11 },			// Switch 5
	{ 15, 11 },			// Switch 6
	{ 13, 15 },			// Switch 7
	{ 14, 15 },			// Switch 8
	{ 15, 15 },			// Switch 9
	{ 13, 20 },			// Switch 10
	{ 14, 20 },			// Switch 11
	{ 15, 20 },			// Switch 12
	{ 13, 25 },			// Switch 13
	{ 14, 25 },			// Switch 14
	{ 15, 25 },			// Switch 15
	{ 13, 30 },			// Switch 16
	{ 14, 30 },			// Switch 17
	{ 15, 30 },			// Switch 18
	{ 16, 9  },			// Switch 153
	{ 16, 16 },			// Switch 154
	{ 16, 23 },			// Switch 155
	{ 16, 30 }			// Switch 156
}; 

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------------------------------------------------------------------------
typedef struct{
	int switch_id; 
	char position; 
} Switch;

typedef struct{
	Switch items[NUM_SWITCHES]; 
} Switches_list;


// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
void switchserver(); 

#endif	/* SWITCHES_SERVER_H */

