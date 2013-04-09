#ifndef AI_SERVER_H
#define	AI_SERVER_H

#include "track_data.h"
#include "track_node.h"

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
#define AI_SERVER_NAME                  "ai_server"
#define GET_NEXT_DESTINATION            1
#define POSITIONS_COUNT                 8
#define GROUPS_COUNT                    3

// ----------------------------------------------------------------------------
// Structures
// ----------------------------------------------------------------------------

// Messages
typedef struct{
    // Track data
    track_node *track;
    
    // Train information
    int train_index;
    track_node **target_node;
    track_edge **target_edge;
    int *target_shift;

    // Message info
    int type;
} AI_msg;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------
void init_ai_server(track_node *track,
                    track_node **pos_nodes, track_edge **pos_edges, int *pos_shifts,
                    int *trainA_start, int *trainA_end,
                    int *trainB_start, int *trainB_end);

int Time();

int get_next_grp();

int get_next_pos();

int get_pos_group(int pos_index);

void get_next_destination(
        track_node **pos_nodes, track_edge **pos_edges, int *pos_shifts,
        int train_index,
        int *trainA_start, int *trainA_end,
        int *trainB_start, int *trainB_end,
        track_node **target_node, track_edge **target_edge, int *target_shift);

void ai_server();

#endif	/* AI_SERVER_H */
