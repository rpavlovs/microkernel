#include "userspace.h"

void init_track_layout( char *layout );
void draw_train( int landmark, int direction, int offset, char *layout, track_node *track );
int get_ui_edge_length( track_edge *e, char *layout, track_node *track );

void track_display() {
	// printf( COM2, "track_display: start\n" );
	char layout[LAYOUT_HEIGHT][LAYOUT_WIDTH];
	int sender_tid;
	msg_init_track_disp init_msg;
	track_node *track;
	msg_display_request req;

	RegisterAs( "track_display" );
	// printf( COM2, "track_display: initializing track layout\n" );
	init_track_layout( (char *)layout );
	// printf( COM2, "track_display: listening for init message with track data location\n" );
	Receive( &sender_tid, (char *)&init_msg, sizeof(init_msg) );
	bwassert( init_msg.type == MSG_TYPE_INIT_TRACK_DISP,
		"track_display: first recieved message should be the initialization message" );
	track = init_msg.track;
	Reply( sender_tid, 0, 0 );
	// printf( COM2, "track_display: got track_data location\n" );

	FOREVER {
		// printf( COM2, "track_display: listening for requests..\n" );
		Receive( &sender_tid, (char *)&req, sizeof(init_msg) );

		switch( req.type ) {
		case MSG_TYPE_DISP_TRAIN:
			// printf( COM2, "track_display: show train request recieved\n" );
			draw_train( req.landmark, req.dir, req.offset, (char *)layout, track );
			break;
		case MSG_TYPE_DISP_SWITCH:
			// printf( COM2, "track_display: show switch request recieved\n" );

			break;
		default:
			bwpanic( "track_display: Unknown message recived" );
		}

		Reply( sender_tid, 0, 0 );
	}

}

void draw_train( int landmark, int direction, int offset, char *layout, track_node *track ) {
	// printf( COM2, "draw_train: start\n" );
	track_edge *edge;
	int egde_path_length = 0;
	char ch;

	switch( track[landmark].type ) {
	case NODE_SENSOR:
		bwassert( direction == DIR_AHEAD, "draw_train: cant go curved on a sensor" );
		// printf( COM2, "draw_train: landmark %d is a sensor\n", landmark );
		edge = &track[landmark].edge[DIR_AHEAD];
		// printf( COM2, "draw_train: line %d, col %d\n", edge->ui_line, edge->ui_col );
		ch = *(layout + edge->ui_line*LAYOUT_WIDTH + edge->ui_col);
		// printf( COM2, "draw_train: ch \"%c\"\n", ch );

		break;
	case NODE_BRANCH:
		// printf( COM2, "draw_train: landmark is a branch\n" );
		edge = &track[landmark].edge[direction];
		// printf( COM2, "draw_train: line %d, col %d\n", edge->ui_line, edge->ui_col );
		ch = *(layout + edge->ui_line*LAYOUT_WIDTH + edge->ui_col);
		// printf( COM2, "draw_train: ch \"%c\"\n", ch );

		break;
	case NODE_MERGE:
		bwassert( direction == DIR_AHEAD, "draw_train: cant go curved on a merge" );
		// printf( COM2, "draw_train: landmark is a merge\n" );
		edge = &track[landmark].edge[DIR_AHEAD];
		// printf( COM2, "draw_train: line %d, col %d\n", edge->ui_line, edge->ui_col );
		ch = *(layout + edge->ui_line*LAYOUT_WIDTH + edge->ui_col);
		// printf( COM2, "draw_train: ch \"%c\"\n", ch );

		break;
	case NODE_ENTER:
		bwassert( direction == DIR_AHEAD, "draw_train: cant go curved on an entry" );
		// printf( COM2, "draw_train: landmark is an entry point\n" );
		edge = &track[landmark].edge[DIR_AHEAD];
		// printf( COM2, "draw_train: line %d, col %d\n", edge->ui_line, edge->ui_col );
		ch = *(layout + edge->ui_line*LAYOUT_WIDTH + edge->ui_col);
		// printf( COM2, "draw_train: ch \"%c\"\n", ch );

		break;
	case NODE_EXIT:
		return;
		break;
	default:
		bwpanic( "draw_train: Landmark type in not supported" );
	}

	egde_path_length = get_ui_edge_length( edge, layout, track );
	printf( COM2, "draw_train: landmark %d edge length is %d\n", landmark, egde_path_length );
}

int get_ui_edge_length( track_edge *e, char *layout, track_node *track ) {
	// printf( COM2, "get_ui_edge_length: start\n" );
	char ch, ch_tl, ch_tc, ch_tr, ch_cl, ch_cr, ch_bl, ch_bc, ch_br;
	int dest_line, dest_col;
	int curr_line, curr_col;
	int path_length = 0;
	
	// if( e->dest->reverse->type = NODE_BRANCH
	// 		&& e->dest->reverse->edge[1].dest->num == e->src->num ) {
	// 	dest_line = e->dest->reverse->edge[1].ui_line;
	// 	dest_col = e->dest->reverse->edge[1].ui_col;
	// } else {
		dest_line = e->reverse->ui_line;
		dest_col = e->reverse->ui_col;
	// }

	curr_line = e->ui_line;
	curr_col = e->ui_col;

	// printf( COM2, "get_ui_edge_length: src (%d, %d) dest(%d, %d)\n",
		// curr_line, curr_col, dest_line, dest_col );


	while( curr_col != dest_col || curr_line != dest_line ) {
		++path_length;
		// printf( COM2, "get_ui_edge_length: length %d ", path_length );
			
		ch = *(layout + curr_line*LAYOUT_WIDTH + curr_col);
		ch_tl = *(layout + (curr_line-1)*LAYOUT_WIDTH + curr_col-1);
		ch_tc = *(layout + (curr_line-1)*LAYOUT_WIDTH + curr_col);
		ch_tr = *(layout + (curr_line-1)*LAYOUT_WIDTH + curr_col+1);
		ch_cl = *(layout + curr_line*LAYOUT_WIDTH + curr_col-1);
		ch_cr = *(layout + curr_line*LAYOUT_WIDTH + curr_col+1);
		ch_bl = *(layout + (curr_line+1)*LAYOUT_WIDTH + curr_col-1);
		ch_bc = *(layout + (curr_line+1)*LAYOUT_WIDTH + curr_col);
		ch_br = *(layout + (curr_line+1)*LAYOUT_WIDTH + curr_col+1);

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

			// printf( COM2, "(%d, %d)\n", curr_line, curr_col );
		}
	}
	return path_length;
}

void init_track_layout( char *layout ) {
	// printf( COM2, "init_track_layout: start\n" );
	mem_cpy( " __________________________________________________________________________________________________________                ", layout + 1*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                           ______/ 12  _____/ 11                                                           \\____           ", layout + 2*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( " _________________________/        ___/  __________________________________________________________________     \\__        ", layout + 3*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                    _____/4       / ____/           13 \\______                       ______/  10           \\___    \\_      ", layout + 4*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "  _________________/            _/ /                          \\                     /                          \\__   \\     ", layout + 5*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                               /__/                            \\___             ___/                              \\_  \\    ", layout + 6*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                              //                                   \\           /                                    \\  \\   ", layout + 7*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          14 /                                      \\_   |   _/                                      \\  \\  ", layout + 8*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                            /                                         \\  |  /                                         \\  \\ ", layout + 9*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                           /                                           | | |                                           \\  |", layout + 10*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                         9C | | | 9B                                         \\ |", layout + 11*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                            | |/                                              \\|", layout + 12*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                             \\|                                               9|", layout + 13*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                              |                                                |", layout + 14*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                              |                                                |", layout + 15*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                              | 9A                                             |", layout + 16*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                           99 |\\                                              8|", layout + 17*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                          |                                             /| |                                             /|", layout + 18*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                           \\                                           | | |                                            / |", layout + 19*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                            \\                                          | | |                                           /  |", layout + 20*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                             \\                                      __/  |  \\__                                       /  / ", layout + 21*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                              \\                                    /     |     \\                                    _/  /  ", layout + 22*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "   ____________                \\                                __/             \\___                              _/   /   ", layout + 23*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "               \\______       15 \\\\___                       ___/                    \\___                       __/   _/    ", layout + 24*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "  ____________________\\          \\__ \\_____________________/____________________________\\_____________________/     /      ", layout + 25*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                     1 \\_______     \\___                  16                            17                      ___/       ", layout + 26*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "  _____________________________\\        \\______________________________________________________________________/           ", layout + 27*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                              2 \\________            6 \\_____                          _____/ 7                            ", layout + 28*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "  _______________________________________\\___________________\\________________________/__________________________          ", layout + 29*LAYOUT_WIDTH, LAYOUT_WIDTH );
	mem_cpy( "                                           3              18                             5                                 ", layout + 30*LAYOUT_WIDTH, LAYOUT_WIDTH );
	// printf( COM2, "init_track_layout: end\n" );

	// printf( COM2, "\n" );
}
