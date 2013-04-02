#ifndef __TRACK_DISPLAY_H__
#define __TRACK_DISPLAY_H__

#define LAYOUT_WIDTH 125
#define LAYOUT_HEIGHT 30

void track_display();

typedef struct msg_init_track_disp 	msg_init_track_disp;
typedef struct msg_display_request 	msg_display_request;

#define MSG_TYPE_INIT_TRACK_DISP 	1
#define MSG_TYPE_DISP_TRAIN 		2
#define MSG_TYPE_DISP_SWITCH 		3

struct msg_init_track_disp {
	int type;
	track_node *track;
};

struct msg_display_request {
	int type;
	int landmark;
	int dir;
	int offset;
	int switch_id;
	int state;
};

#endif
