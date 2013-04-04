#ifndef TRACK_NODE_H
#define	TRACK_NODE_H

#define MAX_CHARS_PER_EDGE  20
#define POS                 0
#define COL                 1

#define MAX_TRAIN_ID        60

typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
  track_edge *reverse;
  track_node *src, *dest;
  int dist;             /* in millimetres */
  
  int ui_line, ui_col, ui_len; // ui start line, column and length

  //Routing algorithm
  int routers[3];

  //Reservation algorithm
  int reservers[3];     //TODO: refactor reserved[TRAINS_NUMBER]
  int start[3];         //TODO: refactor start[TRAINS_NUMBER]
  int end[3];           //TODO: refactor start[TRAINS_NUMBER]
};

struct track_node {
  const char *name;
  node_type type;
  int num;              /* sensor or switch number */
  track_node *reverse;  /* same location, but opposite direction */
  track_edge edge[2];
  
  //Dijkstra algorithm
  int index;
  int visited;
  int label;
  track_node* previous;
  int neighbours_count;
  track_node* neighbours[3];
  int distances[3];
};

#endif


