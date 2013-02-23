#include "commonspace.h"

// Character Queue

void init_char_queue(Char_queue *q){
	q->newest = 0;
	q->oldest = 0;
	q->size = 0;
}

void enqueue_char_queue(char c, Char_queue *q){
	assert(q->size != CHAR_QUEUE_SIZE, "Char queue should not overflow");

	//Enqueue the character
	q->size++;
	if(++(q->newest) == CHAR_QUEUE_SIZE) q->newest = 0;
	q->chars[q->newest] = c;
}

void enqueue_str_to_char_queue(char *str, Char_queue *q){
	//TODO: Implement this function!!!
	//assert(q->size != CHAR_QUEUE_SIZE, "Char queue should not overflow");
}

char dequeue_char_queue(Char_queue *q){
	assert(q->size != 0, "Char queue should have items to dequeue");

	//Dequeue the character
	q->size--;
	char c = q->chars[q->oldest];
	if(++(q->oldest) == CHAR_QUEUE_SIZE) q->oldest = 0;
	return c;
}

// Integer Queue

void init_int_queue(Int_queue *q){
	q->newest = 0;
	q->oldest = 0;
	q->size = 0;
}

void enqueue_int_queue(int i, Int_queue *q){
	assert(q->size != INT_QUEUE_SIZE, "Integer queue should not overflow");

	//Enqueue the integer
	q->size++;
	if(++(q->newest) == INT_QUEUE_SIZE) q->newest = 0;
	q->ints[q->newest] = i;
}

int dequeue_int_queue(Int_queue *q){
	assert(q->size != 0, "Task queue should have items to dequeue");

	//Dequeue the integer
	q->size--;
	int i = q->ints[q->oldest];
	if(++(q->oldest) == INT_QUEUE_SIZE) q->oldest = 0;
	return i;
}
