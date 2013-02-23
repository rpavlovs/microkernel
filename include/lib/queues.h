#ifndef __COMMON_QUEUES_H__
#define __COMMON_QUEUES_H__

#define MAX_NUM_TASKS 100 //TODO: Remove this dirty hack!!!

typedef struct {
	char chars[MAX_NUM_TASKS];
	int newest, oldest;
	int size;
} Char_queue;

typedef struct {
	int ints[MAX_NUM_TASKS];
	int newest, oldest;
	int size;
} Int_queue;

void init_char_queue(Char_queue *);

void enqueue_char_queue(char c, Char_queue *q);

void enqueue_str_to_char_queue(char *str, Char_queue *q);

char dequeue_char_queue(Char_queue *q);

void init_int_queue(Int_queue *q);

void enqueue_int_queue(int i, Int_queue *q);

int dequeue_int_queue(Int_queue *q);

#endif
