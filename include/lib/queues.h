#ifndef __COMMON_QUEUES_H__
#define __COMMON_QUEUES_H__

#define CHAR_QUEUE_SIZE		4000
#define INT_QUEUE_SIZE		1000
#define MSG_QUEUE_SIZE		100

typedef struct {
	char chars[CHAR_QUEUE_SIZE];
	int newest, oldest;
	int size;
} Char_queue;

typedef struct {
	int ints[INT_QUEUE_SIZE];
	int newest, oldest;
	int size;
} Int_queue;

/////////////////////////////////////////////////////////////////////
//
// Character Queue in round buffer
//
/////////////////////////////////////////////////////////////////////

void init_char_queue( Char_queue * );

void char_queue_push( char c, Char_queue *q );

void char_queue_push_str( char *str, Char_queue *q );

char char_queue_pop( Char_queue *q );

char char_queue_pop_back( Char_queue *q );

char char_queue_front( Char_queue *q );

int char_queue_size( Char_queue *q );

// wrappers

int char_queue_peek_str( Char_queue *q, char *str, int len );

int char_queue_pop_word( Char_queue *q, char *str, int len );

int char_queue_pop_str( Char_queue *q, char *str, int len );


/////////////////////////////////////////////////////////////////////
//
// Integer Queue
//
/////////////////////////////////////////////////////////////////////

void init_int_queue( Int_queue *q );

void enqueue_int_queue( int i, Int_queue *q );

int dequeue_int_queue( Int_queue *q );
#endif
