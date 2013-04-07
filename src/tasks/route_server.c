#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Route Server
// -----------------------------------------------------------------------------------------------------------------------------------------------
void init_routing( track_node *track, int train_index ){
    int i, j;
    
    for(i = 0; i < TRACK_MAX; i++){
        track[i].visited = 0;
        track[i].label = INFINITY;
        track[i].previous = 0;
        
        for(j = 0; j < 2; j++){
            track[i].edge[j].routers[train_index] = 0;
        }
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

        //Calculate new label by Dijkstra
        node_label = node->label;
        calc_label = node_label + node->distances[i];
        neighbour_label = node->neighbours[i]->label;

        //Include neighbour in route
        if( calc_label < neighbour_label ){
            node->neighbours[i]->label = calc_label;
            node->neighbours[i]->previous = node;
        }
    }
}

void update_labels_complex(int train_index, track_node *node, int node_shift, int avoid_routed ){
    //Utility variables
    int i, node_label, calc_label, neighbour_label;
    track_edge *edge;

    //Updating label of each node neighbour
    for( i = 0; i < node->neighbours_count; i++ ){

		// RESERVATION AVOIDANCE ////////////////////////////////////
		// We are able to get an edge
		if( avoid_routed ){
		    if(get_edge_by_nodes( node, node->neighbours[i], &edge )){
		        // If edge is free from routing
		        if( edge_has_reservation_conflict( 
					train_index, edge, node_shift, edge->dist - 1 ) ){
		            //printf("Edge is routed! SRC: %s; DST: %s\n",
		                    //edge->src->name, edge->dest->name);
		            continue;
		        }
		    }
		}
        
		// Dijkstra /////////////////////////////////////////////////
        //Calculate new label by Dijkstra
        node_label = node->label;
        calc_label = node_label + node->distances[i];
        neighbour_label = node->neighbours[i]->label;

        //Include neighbour in route
        if( calc_label < neighbour_label ){
            node->neighbours[i]->label = calc_label;
            node->neighbours[i]->previous = node;
        }
    }
}

//TODO:
// A train is regarded as a point!!! TODO: refactor to a train with length
void get_shortest_route(track_node* track, int train_index,
                        track_node* train_node, int train_shift,
                        track_node* target_node, int target_shift,
                        char* switches,
                        int* route_found, track_node** route, 
                        int* route_length, track_edge** edges,
                        int avoid_routed){
    //printf("get_shortest_route: ENTERED\n");

    // Initialization /////////////////////////////////////////////////////////
    // Utility variables
    int i, min_label, switch_num, node_shift;
    track_node *min_node;
    track_node *temp_node;
    track_edge *temp_edge;
    track_node *temp_route[TRACK_MAX];

    //Initialization
    init_routing( track, train_index );
    train_node->label = 0;
    *route_found = 0;
    *route_length = 0;

    ////printf("INITIALIZED\n");

    // Main loop //////////////////////////////////////////////////////////////
    while( exist_unvisited( track ) ){        
        // Iteration initialization
        get_min_label( track, &min_node, &min_label );
        min_node->visited = 1;
        
        //printf("MIN LABEL: %s\n", min_node->name );

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

		// Adjust node shift
		if(min_node == train_node){
			node_shift = train_shift;
		}
		else{
			node_shift = 0;
		}

        // Update neighbours
        update_labels_complex( train_index, min_node, node_shift, avoid_routed );
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
                    //printf("STRAIGHT SWITCH is found: %d\n", switch_num);
                }
                
                else if( route[i]->edge[DIR_CURVED].dest == route[i + 1] ){
                    //Adjust switch to curved
                    switches[get_switch_index( switch_num )] = SWITCH_CURVE_POS;
                    //printf("CURVED SWITCH is found: %d\n", switch_num);
                }
            }
        }

        // Constructing edges pointers array //////////////////////////////////
        //printf("CONSTRUCTING EDGES ARRAY\n");
        for( i = 0; i < *route_length - 1; i++ ){
            if(!get_edge_by_nodes(
                route[i], route[i + 1],
                &edges[i])){
                //printf("Edge retrieval failed. N01: %s; N02: %s\n",
                        //route[i]->name, route[i+1]->name);
            }else{
                //printf("Edge retrieval succeeded.\n");
            }
        }
        
        // Marking the route as routed :) /////////////////////////////////////
        for( i = 0; i < *route_length - 1; i++ ){
            temp_edge = edges[i];
            if(temp_edge != 0){
                temp_edge->routers[train_index] = 1;
            }
        }
        
        // Printing the route /////////////////////////////////////////////////
        //printf("PRINTING THE ROUTE\n");
        //printf("Route length is: %d\n", *route_length);
        for(i = 0; i < *route_length; i++){
            //printf("%s; ", route[i]->name);
        }
    }
    
    //printf("\nget_shortest_route: EXITED\n");
}

void route_server() {
	bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, "ROUTE_SERVER: enters" );
	RegisterAs( ROUTE_SERVER_NAME );
	
	// Data structures
	int sender_tid;
	Route_msg route_msg;
	
	while(1) {
		bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, "ROUTE_SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &route_msg, sizeof( route_msg )  );
		bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, "ROUTE_SERVER: received request [ sender_tid: %d ]", sender_tid );

		switch( route_msg.type ){
			//This message can arrive from:
			//	Train A
			//	Train B
			//	Train AI
			case GET_SHORTEST_ROUTE_MSG:

                                // Try to get a route avoiding routed edges
                                get_shortest_route(
                                    route_msg.track, route_msg.train_index,
                                    route_msg.current_landmark, route_msg.train_shift,
                                    route_msg.target_node, route_msg.target_shift,
                                    route_msg.switches,
                                    route_msg.route_found, route_msg.landmarks,
                                    route_msg.num_landmarks, route_msg.edges,
                                    1);

								//bwprintf(COM2, "ROUTE_FOUND value is: %d\n", *(route_msg.route_found));
                                
                                // If a route is not found, try without avoiding
                                /*if(!(*(route_msg.route_found))){
									bwprintf(COM2, "FIND ROUTE WITHOUT ROUTING AVOIDANCE!!!\n");
                                    get_shortest_route(
                                    route_msg.track, route_msg.train_index,
                                    route_msg.current_landmark, route_msg.train_shift,
                                    route_msg.target_node, route_msg.target_shift,
                                    route_msg.switches,
                                    route_msg.route_found, route_msg.landmarks,
                                    route_msg.num_landmarks, route_msg.edges,
                                    0);
                                }*/

				bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, "ROUTE_SERVER: Replying [ sender_tid: %d ]", sender_tid );
				Reply( sender_tid, 0, 0 );

				break;

			default:
				bwdebug( DBG_SYS, ROUTE_SRV_DEBUG_AREA, "ROUTE_SERVER: Invalid request. [type: %d]", route_msg.type );
				break;
		}
	}
}
