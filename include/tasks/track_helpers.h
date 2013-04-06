#ifndef TRACK_HELPERS_H
#define	TRACK_HELPERS_H

#include "track_data.h"
#include "track_node.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------------------------------------------------------------------------
void init_track( track_node* track );
int get_switch_index( int switch_num );
int get_switch_num( int switch_index );
void get_node_neighbours( track_node *node );
void init_node_neighbours( track_node *node );
int get_edge_by_nodes(
        track_node* src, track_node* dest,
        track_edge **edge);
int edge_is_routed( track_edge* edge );
int edge_is_reserved( int train_index, track_edge* edge );
int edge_has_reservation_conflict(
        int train_index,
        track_edge* edge,
        int reservation_start, int reservation_end );

#endif	/* TRACK_HELPERS_H */
