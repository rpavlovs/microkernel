#include <userspace.h>

// ----------------------------------------------------------------------------
// Utility functions
// ----------------------------------------------------------------------------

// Returns minimum of two integers
int min(int a, int b){
    if(a <= b){
        return a;
    }
    else{
        return b;
    }
}

// ----------------------------------------------------------------------------
// Trees functions
// ----------------------------------------------------------------------------

// Generates trees of possible train routes
void generate_trees_acc(track_node* seed_node, int iteration,
                track_node **nodes_tree, track_edge **edges_tree){
    int i;
    track_node *temp_node;
    track_edge *temp_edge;
    
    // Iterate neighbours
    for(i = 0; i < seed_node->neighbours_count; i++){
        // Put node to an appropriate positions
        temp_node = seed_node->neighbours[i];
        
        // We don't need to include reverse nodes into trees
        if(temp_node != seed_node->reverse){
            nodes_tree[iteration * TREE_MAX_CHILDREN + i] = temp_node;

            // Put edge to an appropriate positions
            get_edge_by_nodes(seed_node, temp_node, &temp_edge);
            edges_tree[iteration * TREE_MAX_CHILDREN + i] = temp_edge;

            // Recurse on child nodes
            if(iteration == 0){
                generate_trees_acc(temp_node, iteration + i + 1,
                        nodes_tree, edges_tree);
            }
        }
    }
}

// Reverses all nodes and edges inside route trees
void reverse_trees(track_node **nodes_tree, track_edge **edges_tree){
    int i;
    track_node* temp_node;
    track_edge* temp_edge;
    
    for(i = 0; i < TREE_LENGTH; i++){
        // Reverse node
        if(nodes_tree[i] != 0){
            temp_node = nodes_tree[i]->reverse;
            nodes_tree[i] = temp_node;
        }
        
        // Reverse edge
        if(edges_tree[i] != 0){
            temp_edge = edges_tree[i]->reverse;
            edges_tree[i] = temp_edge;
        }
    }
}

// Initializes nodes and edges route trees
void init_trees(track_node **nodes_tree, track_edge **edges_tree){
    int i;
    
    for(i = 0; i < TREE_LENGTH; i++){
        nodes_tree[i] = 0;
        edges_tree[i] = 0;
    }
}

// ----------------------------------------------------------------------------
// Route functions
// ----------------------------------------------------------------------------

//Returns:
//      0 - when the previous node WAS NOT found
//      1 - when the previous node WAS found
//Notes:
//      1. Relies on a route
//      2. Will glitch on reverses because there is no edge
int get_previous_node(
        track_node **route, int route_length,
        track_node *node, int *forest,
        track_node **nodes_tree, track_edge **edges_tree){
    
    int i;

    // Node is the first one in the route
    if(route[0] == node){
        generate_trees_acc(node->reverse, 0, nodes_tree, edges_tree);
        reverse_trees(nodes_tree, edges_tree);
        *forest = 1;
        return 1;
    }
    
    // Find the node in the route
    for(i = 1; i < route_length; i++){
        if(route[i] == node){
            // Set up the node
            nodes_tree[0] = route[i - 1];
            
            // Set up the edge
            if(!get_edge_by_nodes(
                nodes_tree[0], node,
                &edges_tree[0])){
				bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "get_previous_node: EDGE FAILED. SRC: %s; DST: %s\n", nodes_tree[0]->name, node->name );
                return 0;
            }
            
            // Trees were created?
            *forest = 0;
            
            // Node is found
            return 1;
        }
    }
    
    // If the node is in the route,
    // it should never get here
	bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "IMPOSSIBLE!!! get_previous_node FAILED. NODE: %s\n", node->name);
    return 0;
}

//Returns:
//      0 - when the route has ended
//      1 - when the next_node is found
//Notes:
//      1. Relies on a given route
//      2. Won't provide any node outside the given route
int get_next_node(
        track_node **route, int route_length,
        track_node *node, track_node **next_node ){
    
    int i;
    
	bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "get_next_node: NODE: %s\n", node->name);

    // Node is the last one in the route
    if(route[route_length - 1] == node){
        *next_node = 0;
        return 0;
    }
    
    // Find the next node in the route
    for( i = 0; i < route_length - 1; i++ ){
        if(route[i] == node){
            *next_node = route[i + 1];
            return 1;
        }
    }
    
    // Should never get here
    return 0;
}

// ----------------------------------------------------------------------------
// Reservation functions
// ----------------------------------------------------------------------------

// Prints all reservations of a given edge
void print_edge_reservations(track_edge *edge){
    /*printf("PRINTING EDGE RESERVATION... ");
    printf("SOURCE: %s; DEST: %s; DIST: %d\n", edge->src->name, edge->dest->name, edge->dist);
	bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "SOURCE: %s; DEST: %s; DIST: %d\n", edge->src->name, edge->dest->name, edge->dist );*/
	bwprintf( COM2, "SOURCE: %s; DEST: %s; DIST: %d\n", edge->src->name, edge->dest->name, edge->dist );
    int i;
    for(i = 0; i < 3; i++){
        //bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "Reserver: %d; start: %d; end: %d;\n", edge->reservers[i], edge->start[i], edge->end[i]);
		bwprintf( COM2, "Reserver: %d; start: %d; end: %d;\n", edge->reservers[i], edge->start[i], edge->end[i]);
    }
}

// Returns:
//      0 - if the given edge IS NOT reserved by the train in the beginning
//      1 - if the given edge IS reserved by the train in the beginning
int edge_start_reserved(int train_index, track_edge *edge){
    //If there is a reservation by the train
    if(edge->reservers[train_index]){
        //If reservation starts in the beginning
        if(edge->start[train_index] < SAFE_DELTA){
            return 1;
        }
    }
    
    return 0;
}

// Returns:
//      0 - if the given edge IS NOT reserved by the train in the end
//      1 - if the given edge IS reserved by the train in the end
int edge_end_reserved(int train_index, track_edge *edge){
    //If there is a reservation by the train
    if(edge->reservers[train_index]){
        //If reservation starts in the beginning
        if(edge->end[train_index] > edge->dist - SAFE_DELTA - 1){
            return 1;
        }
    }
    
    return 0;
}

//Returns:
//      0 - when stopping route COULDN'T BE reserved
//      1 - when stopping route WAS reserved
int check_stopping_edge(
        int train_index,
        track_edge* edge,
        int reservation_start, int reservation_end ){
    
    // Utility variables
    int i, reverse_start, reverse_end;
    
    // Check input parameters
    if((train_index < 0 || train_index > 2) ||
        (reservation_start < 0 || reservation_end > edge->dist - 1)){
        bwprintf( COM2, "Input parameters of check_stopping_edge are wrong!!!\n");
        return 0;
    }
    
    // Check if there are any contradictory reservations on this route
    for( i = 0; i < 3; i++ ){
        if(
                (reservation_start <= edge->start[i] && edge->start[i] <= reservation_end) ||
                (reservation_start <= edge->end[i] && edge->end[i] <= reservation_end) ||
                (edge->start[i] <= reservation_start && reservation_end <= edge->end[i])
          ){
            bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "Contradictory reservations are found in check_stopping_edge!\n");
            return 0;
        }
    }
    
    // If it got here - it should be possible to reserve the edge
    // Reserve the edge
    edge->reservers[train_index] = 1;
    edge->start[train_index] = reservation_start;
    edge->end[train_index] = reservation_end;
    
    //Reserve reverse edge
    reverse_start = (edge->dist - 1) - reservation_start;
    reverse_end = (edge->dist - 1) - reservation_end;
    
    edge->reverse->reservers[train_index] = 1;
    edge->reverse->start[train_index] = reverse_start;
    edge->reverse->end[train_index] = reverse_end;
    
    //print_edge_reservations(edge);
    
    return 1;
}

//Returns:
//      0 - when node is a brach and COULDN'T BE safe-reserved
//      1 - otherwise :)
int check_node_for_branch(
        int train_index,
        track_node *node, track_edge *edge, int strict){
    int j;
    track_edge *temp_edge;
    
    // If we encounter a BRANCH
    if(node->type == NODE_BRANCH){
        // If the edge is reserved in the beginning
        if(edge_start_reserved(train_index, edge)){
            // Find other edge of a branch
            for( j = 0; j < 2; j++ ){
                temp_edge = &(node->edge[j]);
                // Other edge of a branch is found
                if(edge != temp_edge ){
                    if(!check_stopping_edge(
                        train_index, temp_edge,
                        0, SAFE_DELTA) && strict){
                        bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "check_node_for_branch: check_stopping_edge FAILED\n");
                        return 0;
                    }
                }
            }
        }
    }
    
    return 1;
}

//Returns:
//      0 - when node is a merge and COULDN'T BE safe-reserved
//      1 - otherwise :)
int check_node_for_merge(
        int train_index,
        track_edge *edge, track_node *node, int strict){
    int j;
    track_edge *temp_edge;
    
    // If we encounter a MERGE
    if( node->type == NODE_MERGE ){
        // If the edge is reserved in the beginning
        if(edge_end_reserved(train_index, edge)){
            // Find other edge
            for( j = 0; j < 2; j++ ){
                temp_edge = &(node->reverse->edge[j]);
                // Other edge of a merge is found
                if( edge != temp_edge->reverse ){
                    if(!check_stopping_edge(
                        train_index, temp_edge->reverse,
                        temp_edge->reverse->dist - SAFE_DELTA,
                        temp_edge->reverse->dist - 1) && strict){
                        bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "check_node_for_merge: check_stopping_edge FAILED\n");
                        return 0;
                    }
                }
            }
        }
    }
    
    return 1;
}

//Returns:
//      0 - when reverse space WAS NOT reserved
//      1 - when reverse space WAS reserved
//Notes:
//      1. Very strict. Emergency stop if tree fails
int check_forward_tree(
        int train_index, track_node *train_node,
        int reservation, int reservation_limit, int strict){

    // If reverse delta is already violated - emergency stop
    /*if(reservation > reservation_limit && strict){
        //printf("RESERVATION_LIMIT is violated in check_forward_tree!!!\n");
        return 0;
    }*/

    // Variables
    int i;
    int res_start, res_end, res_happened;
    track_edge *temp_edge;
    track_node *nodes_tree[TREE_LENGTH];
    track_edge *edges_tree[TREE_LENGTH];

    // Initialization
    i = 0;
    res_start = 0;
    res_end = 0;
    res_happened = 1;
    init_trees(nodes_tree, edges_tree);
    generate_trees_acc(train_node, 0, nodes_tree, edges_tree);

    // Iterate trees
    for(i = 0; i < TREE_LENGTH; i++){
        // If node exists
        if(nodes_tree[i] != 0 && reservation > 0){
            // Check the edge
            temp_edge = edges_tree[i];
            res_start = 0;
            res_end = min( temp_edge->dist, reservation ) - 1;

            if(!check_stopping_edge(
                train_index, temp_edge,
                res_start, res_end )){
                // If reservation is strict
                if(strict){
                        res_happened = 0;
                }
            }
        }

        // If end of tree layer is reached
        if(i == TREE_MAX_CHILDREN - 1 || i == (TREE_LENGTH - 1)){
            // Update the variables
            reservation -= min( temp_edge->dist, reservation );
        }
    }
    
    if(res_happened){
        return 1;
    }
    else{
        bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "Reverse reservation failed in check_forward_tree!!!\n");
        return 0;
    }
}

//Returns:
//      0 - when reverse space WAS NOT reserved
//      1 - when reverse space WAS reserved
//Notes:
//      1. Very strict. Emergency stop if tree fails
int check_backward_tree(
        int train_index, track_node *train_node,
        int reservation, int reservation_limit, int strict){

    // If reverse delta is already violated - emergency stop
    if(reservation > reservation_limit && strict){
        //printf("RESERVATION_LIMIT is violated in check_backward_tree!!!\n");
        return 0;
    }

    // Variables
    int i;
    int res_start, res_end, res_happened;
    track_edge *temp_edge;
    track_node *nodes_tree[TREE_LENGTH];
    track_edge *edges_tree[TREE_LENGTH];

    // Initialization
    i = 0;
    res_start = 0;
    res_end = 0;
    res_happened = 1;
    init_trees(nodes_tree, edges_tree);
    generate_trees_acc(train_node->reverse, 0, nodes_tree, edges_tree);
    reverse_trees(nodes_tree, edges_tree);

    // Iterate trees
    for(i = 0; i < TREE_LENGTH; i++){
        // If node exists
        if(nodes_tree[i] != 0 && reservation > 0){
            // Check the edge
            temp_edge = edges_tree[i];
            res_start = temp_edge->dist - min( reservation, temp_edge->dist );
            res_end =  temp_edge->dist - 1;

            if(!check_stopping_edge(
                train_index, temp_edge,
                res_start, res_end ) && strict){
                res_happened = 0;
            }
        }

        // If end of tree layer is reached
        if(i == TREE_MAX_CHILDREN - 1 || i == (TREE_LENGTH - 1)){
            // Update the variables
            reservation -= min( temp_edge->dist, reservation );
        }
    }
    
    if(res_happened){
        return 1;
    }
    else{
        bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "Reverse reservation failed in check_backward_tree!!!\n");
        return 0;
    }
}

//Returns:
//      0 - when stop WAS NOT reserved
//      1 - when stop WAS reserved (Always returns 1)
int check_stop(
        int train_index,
        track_node *train_node, track_edge *train_edge, int train_offset,
        int reserve_forward, int reserve_backward){
    
    // General variables
    // int i, j;
    int min_forward, min_backward;              //temp variables
    int leftover_forward, leftover_backward;    //for saving leftovers
    int reservation_start, reservation_end;     //for checking edges
    track_node *node;                           //for route-iteration
    track_edge *edge;
    
    ///////////////////////////////////////////////////////////////////////////
    // Preparing to check the first edge///////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: first edge checking...\n");
    
    leftover_forward = reserve_forward;
    leftover_backward = reserve_backward;
    node = train_node;
    edge = train_edge;
    
    // Check the current train edge
    min_forward = min( leftover_forward, edge->dist - train_offset );
    min_backward = min( leftover_backward, train_offset );
    
    reservation_start = train_offset - min_backward;
    reservation_end = train_offset + min_forward - 1;
    
    check_stopping_edge(
        train_index, edge,
        reservation_start, reservation_end );
    
    // Update variables
    leftover_forward -= min_forward;
    leftover_backward -= min_backward;
    
    ///////////////////////////////////////////////////////////////////////////
    // Check all other edges forward //////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: forward edge checking...\n");
    node = train_edge->dest;
    edge = train_edge;
    
    if(leftover_forward > 0){
        check_forward_tree(
            train_index, node,
            leftover_forward, TRAIN_LENGTH, 0);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Process special cases like merges and branches /////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: special cases checking...\n");
    node = train_edge->dest;
    edge = train_edge;
    
    check_node_for_branch(
        train_index,
        train_node, train_edge, 0);

    check_node_for_merge(
        train_index,
        train_edge, node, 0);
    
    ///////////////////////////////////////////////////////////////////////////
    // Check all other edges backward /////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: backward edge checking...\n");
    
    if(leftover_backward > 0){
        check_backward_tree(
            train_index, train_node,
            leftover_backward, TRAIN_LENGTH, 0);
    }
    
    // Finally!!! :)
    return 1;
}

//Returns:
//      0 - when stop WAS NOT reserved
//      1 - when stop WAS reserved
int check_short_stopping_route(
        int train_index,
        track_node *train_node, track_edge *train_edge, int train_offset,
        int reserve_forward, int reserve_backward){
    
    // General variables
    // int i, j;
    int min_forward, min_backward;              //temp variables
    int leftover_forward, leftover_backward;    //for saving leftovers
    int reservation_start, reservation_end;     //for checking edges
    track_node *node;                           //for route-iteration
    track_edge *edge;
    
    ///////////////////////////////////////////////////////////////////////////
    // Preparing to check the first edge///////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: first edge checking...\n");
    
    leftover_forward = reserve_forward;
    leftover_backward = reserve_backward;
    node = train_node;
    edge = train_edge;
    
    // Check the current train edge
    min_forward = min( leftover_forward, edge->dist - train_offset );
    min_backward = min( leftover_backward, train_offset );

    reservation_start = train_offset - min_backward;
    reservation_end = train_offset + min_forward - 1;

    if(!check_stopping_edge(
        train_index, edge,
        reservation_start, reservation_end )){
        bwprintf( COM2, "RESERVATION ALGORITHM: SHORT STOPPING ROUTE: FIRST EDGE: check_stopping_edge FAILED. Res_start: %d, Res_end: %d\n", 
					reservation_start, reservation_end );
        return 0;
    }

    // Update variables
    leftover_forward -= min_forward;
    leftover_backward -= min_backward;
    
    ///////////////////////////////////////////////////////////////////////////
    // Check all other edges forward //////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: forward edge checking...\n");
    node = train_edge->dest;
    edge = train_edge;
    
    if(leftover_forward > 0){
        if(!check_forward_tree(
            train_index, node,
            leftover_forward, TRAIN_LENGTH, 1)){
	        bwprintf( COM2, "RESERVATION ALGORITHM: SHORT STOPPING ROUTE: FORWARD TREE: reservation failed!!!\n" );
			return 0;
		}
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Process special cases like merges and branches /////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: special cases checking...\n");
    node = train_edge->dest;
    edge = train_edge;
    
    if(!check_node_for_branch(
        train_index,
        train_node, train_edge, 1)){
        bwprintf( COM2, "RESERVATION ALGORITHM: SHORT STOPPING ROUTE: BRANCH: reservation failed!!!\n" );
		return 0;
	}

    if(!check_node_for_merge(
        train_index,
        train_edge, node, 1)){
        bwprintf( COM2, "RESERVATION ALGORITHM: SHORT STOPPING ROUTE: MERGE: reservation failed!!!\n" );
		return 0;
	}
    
    ///////////////////////////////////////////////////////////////////////////
    // Check all other edges backward /////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //printf("STOP: backward edge checking...\n");
    
    if(leftover_backward > 0){
        check_backward_tree(
            train_index, train_node,
            leftover_backward, TRAIN_LENGTH, 0);
    }
    
    // Finally!!! :)
    return 1;
}

//Returns:
//      0 - when stopping route WAS NOT reserved
//      1 - when stopping route WAS reserved
//Notes:
//      1. Relies completely on a given route
//      1. ROUTE CANNOT START WITH REVERSE!!! O_O
int check_stopping_route(
        track_node **route, track_edge **route_edges, int route_length,
        int train_index, 
        track_node *train_node, track_edge *train_edge, int train_offset,
        int reserve_forward, int reserve_backward){
    
    // General variables
    int i, j;
    int min_forward, min_backward;              //temp variables
    int leftover_forward, leftover_backward;    //for saving leftovers
    int reservation_start, reservation_end;     //for checking edges
    track_node *node, *next_node, *prev_node;   //for route-iteration
    track_edge *edge;

	//bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "INPUT_INNER: Node: %s; Shift: %d; Res: %d; Route Length: %d\n", train_node->name, train_offset, reserve_forward, route_length);
    
    ///////////////////////////////////////////////////////////////////////////
    // Preparing to check the first edge///////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    leftover_forward = reserve_forward;
    leftover_backward = reserve_backward;
    node = train_node;
    
    if(!get_next_node(
        route, route_length,
        node, &next_node )){
        //bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "FIRST EDGE: get_next_node FAILED\n");
        //bwprintf( COM2, "RESERVATION ALGORITHM: FIRST EDGE: get_next_node FAILED\n");
        return 0;
    }
    
    if(!get_edge_by_nodes(
        node, next_node,
        &edge)){
        //bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "FIRST EDGE: get_edge_by_nodes FAILED\n");
        bwprintf( COM2, "RESERVATION ALGORITHM: FIRST EDGE: get_edge_by_nodes FAILED\n");
        return 0;
    }
    
    // Check the current train edge
    min_forward = min( leftover_forward, edge->dist - train_offset );
    min_backward = min( leftover_backward, train_offset );
    
    reservation_start = train_offset - min_backward;
    reservation_end = train_offset + min_forward - 1;
    
    if(!check_stopping_edge(
        train_index, edge,
        reservation_start, reservation_end )){
        //bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "FIRST EDGE: check_stopping_edge FAILED\n");
        bwprintf( COM2, "RESERVATION ALGORITHM: FIRST EDGE: check_stopping_edge FAILED. Res_start: %d, Res_end: %d\n", reservation_start, reservation_end );
        return 0;
    }
    
    // Update variables
    leftover_forward -= min_forward;
    leftover_backward -= min_backward;
    node = next_node;
    
    ///////////////////////////////////////////////////////////////////////////
    // Check all other edges forward //////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    while(leftover_forward > 0){
        // First part
        // Get next node and check if it is a reverse
        if(get_next_node(
            route, route_length,
            node, &next_node )){
            // Reverse case
            if(node->reverse == next_node){
                if(check_forward_tree(
                    train_index, node,
                    leftover_forward, REVERSE_DELTA, 1)){
                    leftover_forward = 0;
                }
                else{
                    //bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "FORWARD EDGES: check_forward_tree: FAILED. Leftover: %d\n", leftover_forward);
                    bwprintf( COM2, "RESERVATION ALGORITHM: FORWARD EDGES: check_forward_tree for reverse: FAILED. Leftover: %d\n", leftover_forward);
                    return 0;
                }
            }
        }
        else{
			if(check_forward_tree(
				train_index, node,
				leftover_forward, TRAIN_LENGTH, 1)){
				leftover_forward = 0;
			}
			else{
				//bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "FORWARD EDGES: get_next_node FAILED. Leftover: %d\n", leftover_forward);
				bwprintf( COM2, "RESERVATION ALGORITHM: FORWARD EDGES: check_forward_tree for stopping FAILED. Leftover: %d\n", leftover_forward );
				return 0;
			}
        }

        //Second part
        //Reserve an edge
        if(leftover_forward > 0){
            if(!get_edge_by_nodes(
                node, next_node,
                &edge)){
                bwprintf( COM2, "RESERVATION ALGORITHM: FORWARD EDGES: get_edge_by_nodes FAILED\n");
                return 0;
            }

            // Check the edge
            reservation_start = 0;
            reservation_end = min( leftover_forward, edge->dist ) - 1;

            if(!check_stopping_edge(
                train_index, edge,
                reservation_start, reservation_end )){
                bwprintf( COM2, "RESERVATION ALGORITHM: FORWARD EDGES: check_stopping_edge FAILED\n");
                return 0;
            }

            // Update the variables
            leftover_forward -= min( leftover_forward, edge->dist );
            node = next_node;
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Process special cases like merges and branches /////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    track_node *temp_node;
    
    // SPECIAL CASE 01
    // Reserve some safe space on other edge of all branches in the route
    for( i = 0; i < route_length - 1; i++ ){
        // If we encounter a reverse
        if(route[i]->reverse == route[i+1]){
            break;
        }
        
        // If we encounter a BRANCH
        if(route[i]->type == NODE_BRANCH){
            // If the edge is reserved in the beginning
            if(edge_start_reserved(train_index, route_edges[i])){
                // Find other edge of a branch
                for( j = 0; j < 2; j++ ){
                    edge = &(route[i]->edge[j]);
                    // Other edge of a branch is found
                    if(route_edges[i] != edge ){
                        if(!check_stopping_edge(
                            train_index, edge,
                            0, SAFE_DELTA)){
                            bwprintf( COM2, "RESERVATION ALGORITHM: SPECIAL CASE 01 NODE_BRANCH: check_stopping_edge FAILED\n");
                            return 0;
                        }
                    }
                }
            }
        }
    }
    
    // SPECIAL CASE 02
    // Reserve some safe space for all merges in the route
    for( i = 0; i < route_length - 1; i++ ){
        
        get_next_node(
            route, route_length,
            route[i], &temp_node );
        
        if( route[i]->reverse == temp_node ){
            break;
        }
        
        // If we encounter a MERGE
        if( temp_node->type == NODE_MERGE ){
            // If the edge is reserved in the beginning
            if(edge_end_reserved(train_index, route_edges[i])){
                // Find other edge
                for( j = 0; j < 2; j++ ){
                    edge = &(temp_node->reverse->edge[j]);
                    // Other edge of a merge is found
                    if( route_edges[i] != edge->reverse ){
                        if(!check_stopping_edge(
                            train_index, edge->reverse,
                            edge->reverse->dist - SAFE_DELTA,
                            edge->reverse->dist - 1)){
                            bwprintf( COM2, "RESERVATION ALGORITHM: SPECIAL CASE 02 NODE_MERGE: check_stopping_edge FAILED\n");
                            return 0;
                        }
                    }
                }
            }
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Check all other edges backward /////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    track_node *nodes_tree[TREE_LENGTH];
    track_edge *edges_tree[TREE_LENGTH];
    init_trees(nodes_tree, edges_tree);
    int forest;
    int tree_reserved;
    
    node = train_node;
    forest = 0;
    tree_reserved = 0;
    
    while(leftover_backward > 0){
		//bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "INPUT_INNER2: Node: %s; Temp Node: %s;\n", train_node->name, node->name);

        // Get previous edge to check
        if(!get_previous_node(
            route, route_length,
            node, &forest,
            nodes_tree, edges_tree)){
            bwprintf( COM2, "RESERVATION ALGORITHM: BACKWARD EDGES: get_previous_node FAILED\n");
            return 0;
        }

        // Complex case. Trees were created
        if(forest){
             bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "FOREST: complex case\n");
            // Iterate trees
            for(i = 0; i < TREE_LENGTH; i++){
                // If node exists
                if(nodes_tree[i] != 0 && leftover_backward > 0){
                    //printf("FOREST: nodes_tree: %s; Leftover: %d\n", nodes_tree[i]->name, leftover_backward );
                    // Check the edge
                    edge = edges_tree[i];
                    reservation_start = edge->dist - min( leftover_backward, edge->dist );
                    reservation_end =  edge->dist - 1;

                    if(!check_stopping_edge(
                        train_index, edge,
                        reservation_start, reservation_end )){
                        //printf("FOREST: reservation has not happened\n");
                        //TODO: reservation happened
                    }
                }
                
                // If end of tree layer is reached
                if(i == TREE_MAX_CHILDREN - 1 || i == (TREE_LENGTH - 1)){
                    // Update the variables
                    leftover_backward -= min( leftover_backward, edge->dist );
                    //node = prev_node;
                }
            }

			leftover_backward = 0;
        }
        // Simple case. Only one previous node
        else{
             bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "FOREST: simple case\n");
            // Check the edge
            edge = edges_tree[0];
            reservation_start = edge->dist - min( leftover_backward, edge->dist );
            reservation_end =  edge->dist - 1;

            if(!check_stopping_edge(
                train_index, edge,
                reservation_start, reservation_end )){
                bwprintf( COM2, "RESERVATION ALGORITHM: BACKWARD EDGES: check_stopping_edge in previous FAILED\n");
                return 0;
            }

            // Update the variables
            leftover_backward -= min( leftover_backward, edge->dist );
            node = nodes_tree[0];
        }
    }
    
    // Finally!!! :)
    return 1;
}

//Returns:
//      0 - If the route WAS NOT cleared
//      1 - If the route WAS cleared
int free_route( track_node *track, int train_index){
    
    int i, j;
    track_node *node;
    track_edge *edge;

    for(i = 0; i < TRACK_MAX; i++){
        node = &track[i];
        for(j = 0; j < node->neighbours_count; j++){
            if(get_edge_by_nodes(node, node->neighbours[j], &edge)){
                //Forward edge
                edge->reservers[train_index] = 0;
                edge->start[train_index] = -1;
                edge->end[train_index] = -1;

                //Reverse edge
                edge->reverse->reservers[train_index] = 0;
                edge->reverse->start[train_index] = -1;
                edge->reverse->end[train_index] = -1;
            }
        }
    }

	return 1; 
}

//Returns:
//      0 - if the route wasn't reserved
//      1 - if the route was reserved
void reserve_route(
        track_node *track,
        track_node **route, track_edge **route_edges, int route_length,
        int train_index, int train_direction,
        track_node *train_node, track_edge *train_edge, int train_shift,
        int reservation, int *route_is_reserved){

    int reserve_forward, reserve_backward;
    
	// Reservation initialization /////////////////////////////////////////////
    //Clear the route before reservation
    /*if(!free_route(track, train_index)){
        *route_is_reserved = 0;
        return;
    }*/
    
    //Calculate reservation lengths
    //Forward direction
    if( train_direction ){
        reserve_forward = TRAIN_HEAD + reservation;
        reserve_backward = TRAIN_TAIL;
    }
    //Backward direction
    else{
        reserve_forward = TRAIN_TAIL + reservation;
        reserve_backward = TRAIN_HEAD;
    }
    
	// Performing reservation /////////////////////////////////////////////////
    // Reserving stop position
    if(reservation == 0){
		free_route(track, train_index);
		check_stop(
			train_index,
			train_node, train_edge, train_shift,
			reserve_forward, reserve_backward);
        *route_is_reserved = 1;
    }
    // Reserving short stopping route
    else if((route_length == 1) ||
			(route[route_length - 1] == train_node && reservation > 0)){
		free_route(track, train_index);
		*route_is_reserved = check_short_stopping_route(
                                train_index,
                                train_node, train_edge, train_shift,
                                reserve_forward, reserve_backward);

		//If route reservation failed - reserve current train stop
		if(!(*route_is_reserved)){
			free_route(track, train_index);
			check_stop(
                train_index,
                train_node, train_edge, train_shift,
                reserve_forward, reserve_backward);
		}
    }
    // Reserving long stopping route
	else{
		free_route(track, train_index);
        *route_is_reserved = check_stopping_route(
                                route, route_edges, route_length,
                                train_index,
                                train_node, train_edge, train_shift,
                                reserve_forward, reserve_backward);

		//If route reservation failed - reserve current train stop
		if(!(*route_is_reserved)){
			free_route(track, train_index);
			check_stop(
                train_index,
                train_node, train_edge, train_shift,
                reserve_forward, reserve_backward);
		}
	}

    return;
}

// ----------------------------------------------------------------------------
// Reservation server
// ----------------------------------------------------------------------------

void reservation_server(){
	bwdebug( DBG_SYS, RESERVATION_SRV_DEBUG_AREA, "RESERVATION_SERVER: enters" );
	RegisterAs( RESERVATION_SERVER_NAME ); 
	
	// Data structures
	int sender_tid;
	Reservation_msg res_msg;
	
	while(1) {
		bwdebug( DBG_SYS, RESERVATION_SRV_DEBUG_AREA, "RESERVATION_SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &res_msg, sizeof( res_msg )  );

		switch( res_msg.type ){
			bwdebug( DBG_SYS, RESERVATION_SRV_DEBUG_AREA, "RESERVATION_SERVER: received request from [ sender_tid: %d ]", 
				sender_tid );
			//This message can arrive from:
			//	Train A
			//	Train B
			//	Train AI
			case RESERVE_ROUTE_MSG:

				bwdebug( DBG_USR, TEMP2_DEBUG_AREA, "INPUT_OUTER: Node: %s; Shift: %d; Res: %d\n", res_msg.train_node->name, res_msg.train_shift, res_msg.reservation );
				//bwprintf( COM2, "RESERVATION ALGORITHM: Train direction is: %d\n", res_msg.train_direction );

                                reserve_route(
                                    res_msg.track,
                                    res_msg.route, res_msg.route_edges, res_msg.route_length,
                                    res_msg.train_index, res_msg.train_direction,
                                    res_msg.train_node, res_msg.train_edge, res_msg.train_shift,
                                    res_msg.reservation, res_msg.route_is_reserved);

				Reply( sender_tid, 0, 0 );

				break;

			default:
				bwdebug( DBG_SYS, RESERVATION_SRV_DEBUG_AREA, "RESERVATION_SERVER: Invalid request. [type: %d]", res_msg.type );
				break;
		}
	}
}
