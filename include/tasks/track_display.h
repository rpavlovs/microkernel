#ifndef __TRACK_DISPLAY_H__
#define __TRACK_DISPLAY_H__

#define TRACK_WIDTH 				125
#define TRACK_HEIGHT 				30

#define TRACK_POS_LINE				2
#define TRACK_POS_COL				34

void track_display();

typedef struct msg_init_track_disp 	msg_init_track_disp;
typedef struct msg_display_request 	msg_display_request;
typedef struct train_position		train_position;

#define MSG_TYPE_INIT_TRACK_DISP 	1
#define MSG_TYPE_DISP_TRAIN 		2
#define MSG_TYPE_DISP_SWITCH 		3

#define GO_UP						'^'
#define GO_DOWN						'v'
#define GO_LEFT						'<'
#define GO_RIGHT					'>'

struct msg_init_track_disp {
	int type;
	track_node *track;
};

struct msg_display_request {
	int type;
	
	int landmark;
	int dir;
	int offset;
	int train_id;

	int switch_id;
	int state;
};

struct train_position {
	int row;
	int col;
	char dir;
};

#endif
