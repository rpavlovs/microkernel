#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Route Server
// -----------------------------------------------------------------------------------------------------------------------------------------------
void init_dijkstra( track_node *track ){
    int i;
    for(i = 0; i < TRACK_MAX; i++){
        track[i].visited = 0;
        track[i].label = INFINITY;
        track[i].previous = 0;
    }
}

int exist_unvisited( track_node *track ){
	int i;
	for( i = 0; i < TRACK_MAX; i++){
		if(!track[i].visited) {
			return 1;
		}
	}

	return 0;
}

void get_min_label( track_node* track, track_node** min_node, int* min_label ){
    int i;
    *min_label = INFINITY;
    for( i = 0; i < TRACK_MAX; i++){
            if( *min_label > track[i].label && !track[i].visited ){
                    *min_label = track[i].label;
                    *min_node = &track[i];
            }
    }
}

void update_labels( track_node *node ){
    //Utility variables
    int i, node_label, calc_label, neighbour_label;

    //Updating label of each node neighbour
    for( i = 0; i < node->neighbours_count; i++ ){

        //If the neighbour is not reserved by other train
        //if( !(node->neighbours[i]->reserved) ){

            //Calculate new label by Dijkstra
            node_label = node->label;
            calc_label = node_label + node->distances[i];
            neighbour_label = node->neighbours[i]->label;

            //Include neighbour in route
            if( calc_label < neighbour_label ){
                node->neighbours[i]->label = calc_label;
                node->neighbours[i]->previous = node;
            }
        //}
    }
}

//TODO:
// A train is regarded as a point!!! TODO: refactor to a train with length
void get_shortest_route(track_node* track, int* train_direction,
                        track_node* train_node, int train_shift,
                        track_node* target_node, int target_shift,
                        int* switches,
                        int* route_found, track_node** route, 
                        int* route_length, track_edge** edges){
    //printf("ENTERED\n");

    // Initialization /////////////////////////////////////////////////////////
    // Utility variables
    int i, min_label, switch_num;
    track_node* min_node;
    track_node* temp_node;
    track_node* temp_route[TRACK_MAX];

    //Initialization
    init_dijkstra( track );
    train_node->label = 0;
    *route_found = 0;
    *route_length = 0;
    
    //printf("INITIALIZED\n");

    // Main loop //////////////////////////////////////////////////////////////
    while( exist_unvisited( track ) ){        
        // Iteration initialization
        get_min_label( track, &min_node, &min_label );
        min_node->visited = 1;
        
        ////printf("MIN LABEL: %s\n", min_node->name );

        //At least one way to the target is found
        if( min_node == target_node ){
            //printf("ROUTE IS FOUND!!! :)\n");
            *route_found = 1;
            break;
        }

        // All other nodes cannot be reached (direction-caused dead-end)
        if(min_label == INFINITY){
            //printf("ROUTE IS NOT FOUND!!! :(\n");
            *route_found = 0;
            break;
        }

        // Update neighbours
        // init_node_neighbours( min_node );
        update_labels( min_node );
    }

    if( *route_found ){
        // CONSTRUCTING THE ROUTE /////////////////////////////////////////////
        //printf("CONSTRUCTING THE ROUTE\n");
        
        temp_route[*route_length] = min_node;
        *route_length += 1;
        
        while( train_node->index != min_node->index ){
            temp_node = min_node->previous;
            min_node = temp_node;
            
            temp_route[*route_length] = min_node;
            *route_length += 1;
        }
        
        // REVERSING THE ROUTE ////////////////////////////////////////////////
        //printf("REVERSING THE ROUTE\n");
        
        for( i = 0; i < *route_length; i++ ){
            route[*route_length - i - 1] = temp_route[i];
        }
        
        // ADJUSTING SWITCHES /////////////////////////////////////////////////
        //printf("ADJUSTING SWITCHES\n");
        
        for( i = 0; i < *route_length - 1; i++ ){
            if( route[i]->type == NODE_BRANCH ){
                switch_num = route[i]->num;
                
                if( route[i]->edge[DIR_STRAIGHT].dest == route[i + 1] ){
                    //Adjust switch to straight
                    switches[get_switch_index( switch_num )] = SWITCH_STRAIGHT_POS;
                    ////printf("STRAIGHT SWITCH is found: %d\n", switch_num);
                }
                
                else if( route[i]->edge[DIR_CURVED].dest == route[i + 1] ){
                    //Adjust switch to curved
                    switches[get_switch_index( switch_num )] = SWITCH_CURVE_POS;
                    ////printf("CURVED SWITCH is found: %d\n", switch_num);
                }
            }
        }

        // Constructing edges pointers array //////////////////////////////////
        //printf("CONSTRUCTING EDGES ARRAY\n");
        for( i = 0; i < *route_length - 1; i++ ){
            get_edge_by_nodes(
                route[i], route[i + 1],
                edges[i]);
        }
        
        // Printing the route /////////////////////////////////////////////////
        //printf("PRINTING THE ROUTE\n");
        //printf("Route length is: %d\n", *route_length);
        for(i = 0; i < *route_length; i++){
            //printf("%s; ", route[i]->name);
        }
    }
    
    //printf("\nEXITED\n");
}

void route_server() {
	bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, "ROUTE_SERVER: enters" );
	
	// Data structures
	int sender_tid;
	Route_msg route_msg;
	
	while(1) {
		bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, "ROUTE_SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &route_msg, sizeof( route_msg )  );

		switch( route_msg.type ){
			//This message can arrive from:
			//	Train A
			//	Train B
			//	Train AI
			case GET_SHORTEST_ROUTE_MSG:

                                get_shortest_route(
                                    route_msg.track, route_msg.train_direction,
                                    route_msg.current_landmark, route_msg.train_shift,
                                    route_msg.target_node, route_msg.target_shift,
                                    route_msg.switches,
                                    route_msg.route_found,
                                    route_msg.landmarks,
                                    route_msg.num_landmarks,
                                    route_msg.edges);

				Reply( sender_tid, 0, 0 );

				break;

			default:
				bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, 
					"ROUTE_SERVER: Invalid request. [type: %d]", route_msg.type );
				break;
		}
	}
}
