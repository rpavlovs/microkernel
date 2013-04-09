#include <userspace.h>

void init_ai_server(track_node *track,
                    track_node **pos_nodes, track_edge **pos_edges, int *pos_shifts,
                    int *trainA_start, int *trainA_end,
                    int *trainB_start, int *trainB_end){
    // Track initialization
    // Positions nodes
    pos_nodes[0] = &track[1];          // A2
    pos_nodes[1] = &track[13];         // A14
    pos_nodes[2] = &track[86];         // BR4
    pos_nodes[3] = &track[80];         // BR1
    pos_nodes[4] = &track[8];          // A9
    pos_nodes[5] = &track[6];          // A7
    pos_nodes[6] = &track[5];          // A6
    pos_nodes[7] = &track[34];         // C3
    
    // Positions edges
    pos_edges[0] = &(track[1].edge[DIR_AHEAD]);        // A2
    pos_edges[1] = &(track[13].edge[DIR_AHEAD]);       // A14
    pos_edges[2] = &(track[86].edge[DIR_STRAIGHT]);    // BR4
    pos_edges[3] = &(track[80].edge[DIR_STRAIGHT]);    // BR1
    pos_edges[4] = &(track[8].edge[DIR_AHEAD]);        // A9
    pos_edges[5] = &(track[6].edge[DIR_AHEAD]);        // A7
    pos_edges[6] = &(track[5].edge[DIR_AHEAD]);        // A6
    pos_edges[7] = &(track[34].edge[DIR_AHEAD]);       // C3
    
    // Positions shifts
    pos_shifts[0] = 250;
    pos_shifts[1] = 250;
    pos_shifts[2] = 500;
    pos_shifts[3] = 500;
    pos_shifts[4] = 250;
    pos_shifts[5] = 250;
    pos_shifts[6] = 250;
    pos_shifts[7] = 250;
    
    // Trains initialization
    *trainA_start = 6;
    *trainA_end = 6;
    *trainB_start = 2;
    *trainB_end = 2;
}

int get_next_grp(){
    int group_index;
    group_index = Time() % 3;
    return group_index;
}

int get_next_pos(){
    int pos_index;
    pos_index = Time() % 8;
    return pos_index;
}

int get_pos_group(int pos_index){
    if( pos_index >= 0 && pos_index <=2 ){
        return 0;
    }
    else if( pos_index >= 3 && pos_index <= 6){
        return 1;
    }
    else if (pos_index == 7){
        return 2;
    }
    else{
        return -1;
    }
}

void get_next_destination(
        track_node **pos_nodes, track_edge **pos_edges, int *pos_shifts,
        int train_index,
        int *trainA_start, int *trainA_end,
        int *trainB_start, int *trainB_end,
        track_node **target_node, track_edge **target_edge, int *target_shift){
	
    // Utility variables
    int next_pos;
    int next_pos_group;
    int trainA_start_group;
    int trainA_end_group;
    int trainB_start_group;
    int trainB_end_group;
    
    // Initialization
    next_pos = get_next_pos();
    next_pos_group = get_pos_group(next_pos);
    trainA_start_group = get_pos_group(*trainA_start);
    trainA_end_group = get_pos_group(*trainA_end);
    trainB_start_group = get_pos_group(*trainB_start);
    trainB_end_group = get_pos_group(*trainB_end);

	bwprintf( COM2, "AI TESTING 02: next_pos: %d; next_pos_group: %d; tasg: %d; taeg: %d; tbsg: %d; tbeg: %d\n",
				next_pos, next_pos_group, trainA_start_group, trainA_end_group, trainB_start_group, trainB_end_group);
    
    // Get next position for the train ////////////////////////////////////////
    // Process Train A
    if(train_index == 0){
        while(  next_pos_group == trainA_start_group || // Same group is prohibited
                next_pos == *trainB_start ||             // Other train start is prohibited
                next_pos == *trainB_end){                // Other train end is prohibited
            // Update position
            next_pos = get_next_pos();
			next_pos_group = get_pos_group(next_pos);
        }

		// Update train A start-end variables
		*trainA_start = *trainA_end;
		*trainA_end = next_pos;
    }
    // Process Train B
    else{
        while(  next_pos_group == trainB_start_group || // Same group is prohibited
                next_pos == *trainA_start ||             // Other train start is prohibited
                next_pos == *trainA_end){                // Other train end is prohibited
            // Update position
            next_pos = get_next_pos();
			next_pos_group = get_pos_group(next_pos);
        }

		// Update train B start-end variables
		*trainB_start = *trainB_end;
		*trainB_end = next_pos;
    }

	bwprintf( COM2, "AI TESTING 03: next_pos: %d; next_pos_group: %d; tasg: %d; taeg: %d; tbsg: %d; tbeg: %d\n",
		next_pos, next_pos_group, trainA_start_group, trainA_end_group, trainB_start_group, trainB_end_group);
    
    // Process the position ///////////////////////////////////////////////////
	bwprintf( COM2, "AI TESTING 04: TNode: %s; TEdge SRC: %s; TEdge DST: %s; TShift: %d\n",
		(pos_nodes[next_pos])->name, (pos_edges[next_pos])->src->name, (pos_edges[next_pos])->dest->name, pos_shifts[next_pos]);

    *target_node = pos_nodes[next_pos];
    *target_edge = pos_edges[next_pos];
    *target_shift = pos_shifts[next_pos];
}

void ai_server() {
	bwdebug( DBG_SYS, AI_SRV_DEBUG_AREA, "AI_SERVER: enters" );
	RegisterAs( AI_SERVER_NAME ); 

    // Server
    int initialized;
    initialized = 0;

	// Messaging
	int sender_tid;
	AI_msg ai_msg;
        
        // Track
        track_node *pos_nodes[POSITIONS_COUNT];
        track_edge *pos_edges[POSITIONS_COUNT];
        int pos_shifts[POSITIONS_COUNT];
        
        // Train A
        int trainA_start;
        int trainA_end;
        
        // Train B
        int trainB_start;
        int trainB_end;
	
	while(1) {
		bwdebug( DBG_SYS, AI_SRV_DEBUG_AREA, "AI_SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &ai_msg, sizeof( ai_msg )  );

		switch( ai_msg.type ){
			//This message can arrive from:
			//	Train A
			//	Train B
			case GET_NEXT_DESTINATION:
                                if(!initialized){
                                    // Perform initialization
                                    init_ai_server(ai_msg.track,
                                        pos_nodes, pos_edges, pos_shifts,
                                        &trainA_start, &trainA_end,
                                        &trainB_start, &trainB_end);
                                    initialized = 1;
                                    
									bwprintf( COM2, "AI TESTING 01: Track Test: %s; Train: %d; TAS: %d; TAE: %d; TBS: %d; TBE: %d;\n",
										ai_msg.track[0].name, ai_msg.train_index, trainA_start, trainA_end, trainB_start, trainB_end );

                                    // Process request
                                    get_next_destination(
                                        pos_nodes, pos_edges, pos_shifts,
                                        ai_msg.train_index,
                                        &trainA_start, &trainA_end,
                                        &trainB_start, &trainB_end,
                                        ai_msg.target_node, ai_msg.target_edge, ai_msg.target_shift);

									bwprintf( COM2, "AI TESTING 05: Train: %d; TAS: %d; TAE: %d; TBS: %d; TBE: %d;\n",
										ai_msg.train_index, trainA_start, trainA_end, trainB_start, trainB_end );
									bwprintf( COM2, "TNode: %s; TEdge SRC: %s; TEdge DST: %s; TShift: %d\n",
										(*(ai_msg.target_node))->name, (*(ai_msg.target_edge))->src->name, (*(ai_msg.target_edge))->dest->name, *(ai_msg.target_shift));
                                }
                                else{
                                    // Process request
									bwprintf( COM2, "AI TESTING 01: Track Test: %s; Train: %d; TAS: %d; TAE: %d; TBS: %d; TBE: %d;\n",
										ai_msg.track[0].name, ai_msg.train_index, trainA_start, trainA_end, trainB_start, trainB_end );

                                    // Process request
                                    get_next_destination(
                                        pos_nodes, pos_edges, pos_shifts,
                                        ai_msg.train_index,
                                        &trainA_start, &trainA_end,
                                        &trainB_start, &trainB_end,
                                        ai_msg.target_node, ai_msg.target_edge, ai_msg.target_shift);

									bwprintf( COM2, "AI TESTING 05: Train: %d; TAS: %d; TAE: %d; TBS: %d; TBE: %d;\n",
										ai_msg.train_index, trainA_start, trainA_end, trainB_start, trainB_end );
									bwprintf( COM2, "TNode: %s; TEdge SRC: %s; TEdge DST: %s; TShift: %d\n",
										(*(ai_msg.target_node))->name, (*(ai_msg.target_edge))->src->name, (*(ai_msg.target_edge))->dest->name, *(ai_msg.target_shift));                                    
                                }

				Reply( sender_tid, 0, 0 );

				break;

			default:
				bwdebug( DBG_SYS, AI_SRV_DEBUG_AREA, "AI_SERVER: Invalid request. [type: %d]", ai_msg.type );
				break;
		}
	}
}
