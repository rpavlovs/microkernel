#include "commonspace.h"

// Character Queue in round buffer

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

char dequeue_char_queue( Char_queue *q ) {
	assert(q->size != 0, "Char queue should have items to dequeue");

	//Dequeue the character
	q->size--;
	char c = q->chars[q->oldest];
	if(++(q->oldest) == CHAR_QUEUE_SIZE) q->oldest = 0;
	return c;
}

char char_queue_peek( Char_queue *q ) {
	return q->chars[q->oldest];
}

int char_queue_peek_str( Char_queue *q, char *str, int len ) {
	int str_pos = 0;
	int num_peeked = 0;
	int buf_pos = q->oldest;
	while( str_pos < len - 1 && num_peeked < q->size ) {
		str[str_pos++] = q->chars[buf_pos++];
		if( buf_pos == CHAR_QUEUE_SIZE ) buf_pos = 0;
		num_peeked++;
	}
	str[str_pos] = '\0';
	return str_pos;
}

int char_queue_pop_str( Char_queue *q, char *str, int len ) {
	int pos = 0;
	while( pos < len - 1 && q->size > 0 ) {
		str[pos++] = dequeue_char_queue(q);
	}
	str[pos] = '\0';
	return pos;
}

int char_queue_pop_word( Char_queue *q, char *str, int len ) {
	int pos = 0;
	char c;
	while( pos < len - 1 && q->size > 0 ) {
		c = dequeue_char_queue(q);
		if( c == ' ' ) break;
		str[pos++] = c;
	}
	str[pos] = '\0';
	return pos;
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
