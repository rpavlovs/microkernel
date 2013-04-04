#ifndef _ROUTE_SERVER_H__
#define	_ROUTE_SERVER_H__

#include "track_data.h"
#include "track_node.h"
#include "track_helpers.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------
#define ROUTE_SERVER_NAME		"route_server"

#define GET_SHORTEST_ROUTE_MSG		1

//#define TRACK_MAX 144		//TODO: Refactor
#define INFINITY 1000000000	//TODO: Refactor


// -----------------------------------------------------------------------------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------------------------------------------------------------------------

// Messages
typedef struct{
    // Track data
    track_node *track;

    // Current train information
    //int *train_direction; //TODO: DELETE FROM THE MESSAGE!!!
	int train_index;
    track_node *current_landmark;
    int train_shift;

    // Route
    int* route_found;
    track_node **landmarks;  // The landmarks (nodes) to follow to reach to the destination.
    int *num_landmarks;     // The num. of landmarks (including the origin landmark).
    track_edge **edges;      // The edges connecting the landmarks to reach the destination.
    
    // Target information
    track_node *target_node;
    int target_shift;

    // Current track data
    //char *current_switches;
    //char *target_switches;
    char *switches;

    int type;
} Route_msg;

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
int exist_unvisited( track_node *track );
void get_min_label( track_node* track, track_node** min_node, int* min_label );
void update_labels( track_node *node );
void update_labels_complex( track_node *node, int avoid_routed );
void get_shortest_route(track_node* track, int train_index,
                        track_node* train_node, int train_shift,
                        track_node* target_node, int target_shift,
                        char* switches,
                        int* route_found, track_node** route, 
                        int* route_length, track_edge** edges,
                        int avoid_routed);
void route_server();

#endif	/* ROUTE_SERVER_H */
