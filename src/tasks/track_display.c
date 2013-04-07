#include "userspace.h"

void init_track_layout( char *layout );
int traverse_edge( track_edge *e, int length, train_position *train_pos,
		char *layout, track_node *track );
void draw_track( char *layout );
void draw_branch( track_node *branch, int state, char *layout );
void draw_train( int train_id, train_position *train_pos );
void erase_train( train_position *train_pos, char *layout);

void track_display() {
	//printf( COM2, "track_display: start\n" );
	//bwprintf( COM2, "TRACK_DISPLAY: start\n" ); 

	char layout[TRACK_HEIGHT][TRACK_WIDTH];
	int layout_distance;
	int sender_tid;
	int i;
	
	train_position train_posns[MAX_TRAIN_ID+1];
	msg_init_track_disp init_msg;
	track_node *track;
	track_edge *edge;
	msg_display_request req;

	RegisterAs( TRACK_DISPLAY_NAME );
	
	init_track_layout( (char *)layout );
	draw_track( (char *)layout );
	for( i = 0; i < MAX_TRAIN_ID+1; ++i ) {
		train_posns[i].row = 0;
		train_posns[i].col = 0;
	}

	//bwprintf( COM2, "TRACK_DISPLAY: Initializing\n" ); 
	Receive( &sender_tid, (char *)&init_msg, sizeof(init_msg) );
	bwassert( init_msg.type == MSG_TYPE_INIT_TRACK_DISP,
		"track_display: first recieved message should be the initialization message" );
	track = init_msg.track;
	Reply( sender_tid, 0, 0 );
	//bwprintf( COM2, "TRACK_DISPLAY: Finished initialization \n" ); 

	FOREVER {
		//bwprintf( COM2, "TRACK_DISPLAY: Listening for a request \n" ); 
		Receive( &sender_tid, (char *)&req, sizeof(init_msg) );
		//bwprintf( COM2, "TRACK_DISPLAY: Received request [ sender_tid: %d request: %d ] \n", 
		//	sender_tid, req.type ); 

		switch( req.type ) {
		case MSG_TYPE_DISP_TRAIN:
			// printf( COM2, "track_display: show train request recieved\n" );
			 
			bwassert( !(req.dir == DIR_CURVED && track[req.landmark].type != NODE_BRANCH),
				"draw_train: can go curved only on a branch" );

			edge = &track[req.landmark].edge[req.dir];
			layout_distance = (edge->ui_len * req.offset) / edge->dist; 

			if( train_posns[req.train_id].row != 0 && train_posns[req.train_id].col != 0 )
				erase_train( &train_posns[req.train_id], (char *)layout );

			traverse_edge( edge, layout_distance, &train_posns[req.train_id],
				(char *)layout, track );

			draw_train( req.train_id, &train_posns[req.train_id] );

			break;
		case MSG_TYPE_DISP_SWITCH:
			// printf( COM2, "track_display: show switch request recieved\n" );
			bwassert( track[req.switch_id].type == NODE_BRANCH,
				"Has to recieve a branch id" );

			draw_branch( &track[req.switch_id], req.state, (char *)layout );

			break;
		default:
			bwpanic( "track_display: Unknown message recived" );
		}

		Reply( sender_tid, 0, 0 );
	}
}

int traverse_edge( track_edge *e, int length, train_position *train_pos,
		char *layout, track_node *track ) {

	char ch, ch_tl, ch_tc, ch_tr, ch_cl, ch_cr, ch_bl, ch_bc, ch_br;
	int dest_line, dest_col;
	int curr_line, curr_col;
	int length_traversed = 0;
	
	dest_line = e->reverse->ui_line;
	dest_col = e->reverse->ui_col;

	curr_line = e->ui_line;
	curr_col = e->ui_col;

	while( length_traversed < length ) {
		bwassert( !(curr_col == dest_col && curr_line == dest_line),
			"traverse_edge: should not go beyond the end of the edge" );
		++length_traversed;
			
		ch = *(layout + curr_line*TRACK_WIDTH + curr_col);
		ch_tl = *(layout + (curr_line-1)*TRACK_WIDTH + curr_col-1);
		ch_tc = *(layout + (curr_line-1)*TRACK_WIDTH + curr_col);
		ch_tr = *(layout + (curr_line-1)*TRACK_WIDTH + curr_col+1);
		ch_cl = *(layout + curr_line*TRACK_WIDTH + curr_col-1);
		ch_cr = *(layout + curr_line*TRACK_WIDTH + curr_col+1);
		ch_bl = *(layout + (curr_line+1)*TRACK_WIDTH + curr_col-1);
		ch_bc = *(layout + (curr_line+1)*TRACK_WIDTH + curr_col);
		ch_br = *(layout + (curr_line+1)*TRACK_WIDTH + curr_col+1);

		if( curr_col <= dest_col && curr_line <= dest_line ) {
			// printf( COM2, "go right-down " );

			switch( ch ) {
			case '_':
				if( ch_cr == '/' || ch_cr == '_' ) { ++curr_col; }
				else if( ch_br == '\\' ) { ++curr_col; ++curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '/':
				if( ch_bl == '|' ) { --curr_col; ++curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '\\':
				if( ch_cr == '_' ) { ++curr_col; }
				else if( ch_br == '\\' || ch_br == '|' ) { ++curr_col; ++curr_line;	}
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '|':
				if( ch_br == '\\' ) { ++curr_col; ++curr_line; }
				else if( ch_bc == '|' ) { ++curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			default:
				bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
			}
			// printf( COM2, "(%d, %d)\n", curr_line, curr_col );
		} else if( curr_col <= dest_col && curr_line > dest_line ) {
			// printf( COM2, "go right-up " );
			
			switch( ch ) {
			case '_':
				if( ch_cr == '_' || ch_cr == '/' ) { ++curr_col; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '/':
				if( ch_tr == '_' || ch_tr == '/' || ch_tr == '|' ) { ++curr_col; --curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '\\':
				if( ch_tl == '|' || ch_tl == '\\' ) { --curr_line; --curr_col; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '|':
				if( ch_tr == '/' ) { ++curr_col; --curr_line; }
				else if( ch_tc == '|' ) { --curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			default:
				bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
			}
			// printf( COM2, "(%d, %d)\n", curr_line, curr_col );
		} else if( curr_line <= dest_line ) {
			// printf( COM2, "go left-down ch \"%c\" ch_cl \"%c\" ch_bl \"%c\" ch_bc \"%c\" ",
				// ch, ch_cl, ch_bl, ch_bc );

			switch( ch ) {
			case '_':
				if( ch_cl == '\\' || ch_cl == '_' ) { --curr_col; }
				else if( ch_bl == '/' ) { --curr_col; ++curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '/':
				if( ch_cl == '_' ) { --curr_col; }
				else if( ch_bl == '/' || ch_bl == '|' ) { --curr_col; ++curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '\\':
				bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '|':
				if( ch_bc == '|' ) { ++curr_line; }
				else if( ch_bl == '/' ) { --curr_col; ++curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			default:
				bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
			}
			// printf( COM2, "(%d, %d)\n", curr_line, curr_col );
		} else {
			// printf( COM2, "go left-up " );

			switch( ch ) {
			case '_':
				if( ch_cl == '_' || ch_cl == '\\' ) { --curr_col; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '/':
				bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '\\':
				if( ch_tl == '_' || ch_tl == '\\' || ch_tl == '|' ) { --curr_col; --curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			case '|':
				if( ch_tl == '\\' ) { --curr_col; --curr_line; }
				else if( ch_tc == '|' ) { --curr_line; }
				else bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
				break;
			default:
				bwpanic( "get_ui_edge_length: wrong edge ui position coordinates" );
			}
		}
	}

	train_pos->row = curr_line;
	train_pos->col = curr_col;


	if( !(curr_col == dest_col && curr_line == dest_line) ) {
		switch( ch ) {
		case '_':
			train_pos->dir = ( curr_col < dest_col ? GO_RIGHT : GO_LEFT );
			break;
		case '|':
		case '\\':
		case '/':
			train_pos->dir = ( curr_line > dest_line ? GO_UP : GO_DOWN );
		}
	}

	return length_traversed;
}

void draw_track( char *layout ) {
	char buff[TRACK_WIDTH*TRACK_HEIGHT + 100*TRACK_HEIGHT], *ptr;
	int i;

	ptr = buff;
	ptr += saveCursor( ptr );

	for( i = 1; i <= TRACK_HEIGHT; ++i ) {
		ptr += cursorPositioning( ptr, TRACK_POS_LINE + i-1, TRACK_POS_COL );
		ptr += sprintf( ptr, (layout + i*TRACK_WIDTH) );
	}

	ptr += restoreCursor( ptr );

	Putstr( COM2, buff );
}

void draw_branch( track_node *branch, int state, char *layout ) {
	char buff[50];
	char *ptr = buff;

	ptr += saveCursor( ptr );
	ptr += cursorPositioning( ptr,
		TRACK_POS_LINE + branch->edge[state].ui_line - 1,
		TRACK_POS_COL + branch->edge[state].ui_col );
	ptr += sprintf( ptr, "%c",
		*(layout + (branch->edge[state].ui_line)*TRACK_WIDTH + branch->edge[state].ui_col) );
	ptr += cursorPositioning( ptr,
		TRACK_POS_LINE + branch->edge[!state].ui_line - 1,
		TRACK_POS_COL + branch->edge[!state].ui_col );
	ptr += sprintf( ptr, "*" );
	ptr += restoreCursor( ptr );
	
	Putstr( COM2, buff );
}

void draw_train( int train_id, train_position *train_pos) {
	char buff[70];
	char *ptr = buff;

	ptr += saveCursor( ptr );
	//ptr += cursorPositioning( ptr,
	//	TRACK_POS_LINE + train_pos->row - 1, TRACK_POS_COL + train_pos->col );
	ptr += sprintf( ptr, "\033[%d;%dH", 
		TRACK_POS_LINE + train_pos->row - 1, TRACK_POS_COL + train_pos->col ); 

	// Add the color
	switch( train_id ){
		case 43:
			ptr += sprintf( ptr, "\033[36m" ); 
			break;
		case 47:
			ptr += sprintf( ptr, "\033[31m" ); 
			break; 
		case 49:
			ptr += sprintf( ptr, "\033[35m" ); 
			break;
		case 50: 
			ptr += sprintf( ptr, "\033[34m" ); 
			break;
		default: 
			ptr += sprintf( ptr, "\033[33m" ); 
			break;
	}

	ptr += sprintf( ptr, "%c", train_pos->dir );
	ptr += sprintf( ptr, "\033[37m" );			// Return to white text. 
	ptr += restoreCursor( ptr );
	
	Putstr( COM2, buff );
	//if ( train_id == 49 )
	//	bwprintf( COM2, "%d", train_pos->dir ); 
}

void erase_train( train_position *train_pos, char *layout ) {
	char buff[50];
	char *ptr = buff;

	ptr += saveCursor( ptr );
	ptr += cursorPositioning( ptr,
		TRACK_POS_LINE + train_pos->row - 1, TRACK_POS_COL + train_pos->col );
	ptr += sprintf( ptr, "%c", *(layout + (train_pos->row)*TRACK_WIDTH + train_pos->col) );
	ptr += restoreCursor( ptr );
	
	Putstr( COM2, buff );
}

void init_track_layout( char *layout ) {
	mem_cpy( " __________________________________________________________________________________________________________                ", layout + 1*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                           ______/ 12  _____/ 11                                                           \\____           ", layout + 2*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( " _________________________/        ___/  __________________________________________________________________     \\__        ", layout + 3*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                    _____/4       / ____/           13 \\______                       ______/  10           \\___    \\_      ", layout + 4*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "  _________________/            _/ /                          \\                     /                          \\__   \\     ", layout + 5*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                               /__/                            \\___             ___/                              \\_  \\    ", layout + 6*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                              //                                   \\           /                                    \\  \\   ", layout + 7*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          14 /                                      \\_   |   _/                                      \\  \\  ", layout + 8*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                            /                                         \\  |  /                                         \\  \\ ", layout + 9*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                           /                                           | | |                                           \\  |", layout + 10*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                         9C | | | 9B                                         \\ |", layout + 11*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                            | |/                                              \\|", layout + 12*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                             \\|                                               9|", layout + 13*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                              |                                                |", layout + 14*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                              |                                                |", layout + 15*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                              | 9A                                             |", layout + 16*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                           99 |\\                                              8|", layout + 17*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                          |                                             /| |                                             /|", layout + 18*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                           \\                                           | | |                                            / |", layout + 19*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                            \\                                          | | |                                           /  |", layout + 20*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                             \\                                      __/  |  \\__                                       /  / ", layout + 21*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                              \\                                    /     |     \\                                    _/  /  ", layout + 22*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "   ____________                \\                                __/             \\___                              _/   /   ", layout + 23*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "               \\______       15 \\\\___                       ___/                    \\___                       __/   _/    ", layout + 24*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "  ____________________\\          \\__ \\_____________________/____________________________\\_____________________/     /      ", layout + 25*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                     1 \\_______     \\___                  16                            17                      ___/       ", layout + 26*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "  _____________________________\\        \\______________________________________________________________________/           ", layout + 27*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                              2 \\________            6 \\_____                          _____/ 7                            ", layout + 28*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "  _______________________________________\\___________________\\________________________/__________________________          ", layout + 29*TRACK_WIDTH, TRACK_WIDTH );
	mem_cpy( "                                           3              18                             5                                 ", layout + 30*TRACK_WIDTH, TRACK_WIDTH );
}
