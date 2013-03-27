#include <userspace.h>

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------------------------------------------------------------------------

// Single initialization of the track during RTOS initialization
void init_track( track_node* track ){
    int i, j, k;
    for(i = 0; i < TRACK_MAX; i++){
        track_node* node = &track[i];
        
        // Initialization for Dijkstra algorithm
        node->index = i;
        node->neighbours_count = 0;
        init_node_neighbours( node );
        
        // Initialization for Reservation algorithm
        for(j = 0; j < 2; j++){
            node->edge[j].reservers[0] = 0;    //TODO: refactor to train IDs
            node->edge[j].reservers[1] = 0;    //TODO: refactor to train IDs
            node->edge[j].reservers[2] = 0;    //TODO: refactor to train IDs
            
            for(k = 0; k < 3; k++){
                node->edge[j].start[k] = -1;
                node->edge[j].end[k] = -1;
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Switches functions
// -----------------------------------------------------------------------------------------------------------------------------------------------
int get_switch_index( int switch_num ){
    if( switch_num >= 1 && switch_num <= 18 ){
        return switch_num - 1;
    }
    else if( switch_num >= 153 && switch_num <= 156 ){
        return ( switch_num - 135 );
    }
    else{
        return -1;
    }
}

int get_switch_num( int switch_index ){
    if( switch_index >= 0 && switch_index <= 17 ){
        return switch_index + 1;
    }else{
        return ( switch_index + 135 );
    }    
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Neighbours functions
// -----------------------------------------------------------------------------------------------------------------------------------------------
void get_node_neighbours( track_node *node ){
	switch(node->type){
		case NODE_SENSOR:
			node->neighbours_count = 1;
			node->neighbours[0] = node->edge[DIR_AHEAD].dest;
			node->distances[0] = node->edge[DIR_AHEAD].dist;

			break;

		case NODE_BRANCH:
			node->neighbours_count = 2;

			if(node->edge[DIR_STRAIGHT].dist < node->edge[DIR_CURVED].dist)
			{
				node->neighbours[0] = node->edge[DIR_STRAIGHT].dest;
				node->neighbours[1] = node->edge[DIR_CURVED].dest;

				node->distances[0] = node->edge[DIR_STRAIGHT].dist;
				node->distances[1] = node->edge[DIR_CURVED].dist;
			}
			else
			{
				node->neighbours[0] = node->edge[DIR_CURVED].dest;
				node->neighbours[1] = node->edge[DIR_STRAIGHT].dest;

				node->distances[0] = node->edge[DIR_CURVED].dist;
				node->distances[1] = node->edge[DIR_STRAIGHT].dist;
			}

			break;

		case NODE_MERGE:
			node->neighbours_count = 1;
			node->neighbours[0] = node->edge[DIR_AHEAD].dest;
			node->distances[0] = node->edge[DIR_AHEAD].dist;

			break;

		case NODE_ENTER:
			node->neighbours_count = 1;
			node->neighbours[0] = node->edge[DIR_AHEAD].dest;
			node->distances[0] = node->edge[DIR_AHEAD].dist;

			break;

		case NODE_EXIT:
			node->neighbours_count = 0;
			break;

		case NODE_NONE:
			node->neighbours_count = 0;
			break;
	}
}

void init_node_neighbours( track_node* node ){
    switch(node->type){
            case NODE_SENSOR:
                    node->neighbours_count = 2;

                    node->neighbours[0] = node->reverse;
                    node->neighbours[1] = node->edge[DIR_AHEAD].dest;

                    node->distances[0] = 0;
                    node->distances[1] = node->edge[DIR_AHEAD].dist;

                    break;

            case NODE_BRANCH:
                    node->neighbours_count = 2;

                    if(node->edge[DIR_STRAIGHT].dist < node->edge[DIR_CURVED].dist)
                    {
                            node->neighbours[0] = node->edge[DIR_STRAIGHT].dest;
                            node->neighbours[1] = node->edge[DIR_CURVED].dest;

                            node->distances[0] = node->edge[DIR_STRAIGHT].dist;
                            node->distances[1] = node->edge[DIR_CURVED].dist;
                    }
                    else
                    {
                            node->neighbours[0] = node->edge[DIR_CURVED].dest;
                            node->neighbours[1] = node->edge[DIR_STRAIGHT].dest;

                            node->distances[0] = node->edge[DIR_CURVED].dist;
                            node->distances[1] = node->edge[DIR_STRAIGHT].dist;
                    }

                    break;

            case NODE_MERGE:
                    node->neighbours_count = 1;
                    node->neighbours[0] = node->edge[DIR_AHEAD].dest;
                    node->distances[0] = node->edge[DIR_AHEAD].dist;

                    break;

            case NODE_ENTER:
                    node->neighbours_count = 1;
                    node->neighbours[0] = node->edge[DIR_AHEAD].dest;
                    node->distances[0] = node->edge[DIR_AHEAD].dist;

                    break;

            case NODE_EXIT:
                    node->neighbours_count = 1;
                    node->neighbours[0] = node->reverse;
                    node->distances[0] = 0;

                    break;

            case NODE_NONE:
                    node->neighbours_count = 0;

                    break;
    }
}

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Edges functions
// -----------------------------------------------------------------------------------------------------------------------------------------------
// Returns:
//      0 - when the edge IS NOT found
//      1 - when the edge IS found
int get_edge_by_nodes(
        track_node* src, track_node* dest,
        track_edge* edge){
    
    int i;
    for( i = 0; i < 2; i++ ){
        if( src->edge[i].dest == dest ){
            //Edge is found
            edge = &(src->edge[i]);
            return 1;
        }
    }
    
    // Shouldn't get here
    return 0;
}
