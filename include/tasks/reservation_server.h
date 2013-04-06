#ifndef RESERVATION_SERVER_H
#define	RESERVATION_SERVER_H

#include "track_data.h"
#include "track_node.h"
#include "track_helpers.h"

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
#define TRAIN_HEAD 25
#define TRAIN_TAIL 195
#define TRAIN_LENGTH 220
#define SAFE_DELTA 150
#define REVERSE_DELTA 250

#define TREE_LENGTH 12
#define TREE_MAX_CHILDREN 3

#define RESERVATION_SERVER_NAME		"reservation_server"
#define RESERVE_ROUTE_MSG		1

// ----------------------------------------------------------------------------
// Structures
// ----------------------------------------------------------------------------
typedef struct{
    // Track data
    track_node *track;

    // Current train information
    int train_index;
    int train_direction;
    track_node *train_node;
    track_edge *train_edge;
    int train_shift;
    
    // Route
    track_node **route;
    track_edge **route_edges;
    int route_length;
    
    // Reservation
    int reservation;
    int *route_is_reserved;

    int type;
} Reservation_msg;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

// Utility functions //////////////////////////////////////////////////////////

int min(int a, int b);

// Trees functions ////////////////////////////////////////////////////////////

void generate_trees_acc(track_node* seed_node, int iteration,
                track_node **nodes_tree, track_edge **edges_tree);

void reverse_trees(track_node **nodes_tree, track_edge **edges_tree);

void init_trees(track_node **nodes_tree, track_edge **edges_tree);

// Route functions ////////////////////////////////////////////////////////////

int get_previous_node(
        track_node **route, int route_length,
        track_node *node, int *forest,
        track_node **nodes_tree, track_edge **edges_tree);

int get_next_node(
        track_node **route, int route_length,
        track_node *node, track_node **next_node );

// Reservation functions //////////////////////////////////////////////////////

void print_edge_reservations(track_edge *edge);

int edge_start_reserved(int train_index, track_edge *edge);

int edge_end_reserved(int train_index, track_edge *edge);

int check_stopping_edge(
        int train_index,
        track_edge* edge,
        int reservation_start, int reservation_end );

int check_node_for_branch(
        int train_index,
        track_node *node, track_edge *edge, int strict);

int check_node_for_merge(
        int train_index,
        track_edge *edge, track_node *node, int strict);

int check_forward_tree(
        int train_index, track_node *train_node,
        int reservation, int reservation_limit, int strict);

int check_backward_tree(
        int train_index, track_node *train_node,
        int reservation, int reservation_limit, int strict);

int check_stop(
        int train_index,
        track_node *train_node, track_edge *train_edge, int train_offset,
        int reserve_forward, int reserve_backward);

int check_stopping_route(
        track_node **route, track_edge **route_edges, int route_length,
        int train_index, 
        track_node *train_node, track_edge *train_edge, int train_offset,
        int reserve_forward, int reserve_backward);

int free_route( track_node *track, int train_index);

void reserve_route(
        track_node *track,
        track_node **route, track_edge **route_edges, int route_length,
        int train_index, int train_direction,
        track_node *train_node, track_edge *train_edge, int train_shift,
        int reservation, int *route_is_reserved);

// Reservation server /////////////////////////////////////////////////////////

void reservation_server();

#endif	/* RESERVATION_SERVER_H */
