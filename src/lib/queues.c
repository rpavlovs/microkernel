#include "commonspace.h"

/////////////////////////////////////////////////////////////////////
//
// Character Queue in round buffer
//
/////////////////////////////////////////////////////////////////////

void init_char_queue(Char_queue *q){
	q->newest = -1;
	q->oldest = 0;
	q->size = 0;
}

void char_queue_push(char c, Char_queue *q){
	bwassert(q->size != CHAR_QUEUE_SIZE, "char_queue_push: Char queue should not overflow");

	//Enqueue the character
	q->size++;
	if(++(q->newest) == CHAR_QUEUE_SIZE) q->newest = 0;
	q->chars[q->newest] = c;
}

void char_queue_push_str(char *str, Char_queue *q){
	char *ptr = str;
	
	do {
		bwassert( q->size != CHAR_QUEUE_SIZE,
			"char_queue_push_str: Char queue should not overflow" );
		q->size++; 
		
		if( ++(q->newest) >= CHAR_QUEUE_SIZE ) 
			q->newest = 0;
		
		q->chars[q->newest] = *( ptr++ ); 
	} while( *ptr );
}

char char_queue_pop( Char_queue *q ) {
	bwassert( q->size > 0, "char_queue_pop: Char queue should not be empty." ); 

	//Dequeue the character
	q->size--;
	char c = q->chars[q->oldest];
	if(++(q->oldest) == CHAR_QUEUE_SIZE) q->oldest = 0;
	return c;
}

char char_queue_pop_back( Char_queue *q ) {
	bwassert( q->size > 0, "char_queue_pop_back: Char queue should not be empty." );
	
	// Pop the character. 
	q->size--; 
	char c = q->chars[ (q->newest)-- ]; 
	if( q->newest < 0 )
		q->newest = CHAR_QUEUE_SIZE - 1; 
	
	return c; 
}

int char_queue_pop_word( Char_queue *q, char *str, int len ) {
	int pos = 0;
	char c = char_queue_front( q );

	while( q->size > 0 && c == ' ' && c != '\0' ) {
		char_queue_pop( q );
		c = char_queue_front( q );
	}

	while( q->size > 0 && pos < len - 1 && c != ' ' && c != '\0') {
		str[pos++] = char_queue_pop( q );
		c = char_queue_front( q );
	}
	str[pos] = '\0';
	return pos;
}

int char_queue_pop_str( Char_queue *q, char *str, int len ) {
	int pos = 0;
	char c = char_queue_front( q );

	while( q->size > 0 && pos < len - 1 && c != '\0' ) {
		str[pos++] = char_queue_pop( q );
		c = char_queue_front( q );
	}
	str[pos] = '\0';
	return pos;
}

char char_queue_front( Char_queue *q ) {
	bwassert( q->size > 0, "char_queue_front: Char queue should not be empty." );
	return q->chars[q->oldest];
}

int char_queue_peek_str( Char_queue *q, char *str, int len ) {
	int str_pos = 0;
	int num_peeked = 0;
	int buf_pos = q->oldest;
	while( str_pos < len - 1 && num_peeked < q->size ) {
		str[str_pos++] = q->chars[buf_pos++];
		if( buf_pos == CHAR_QUEUE_SIZE ) 
			buf_pos = 0;
		num_peeked++;
	}
	str[str_pos] = '\0';
	return str_pos;
}

int char_queue_size( Char_queue *q ) {
	return q->size;
}

/////////////////////////////////////////////////////////////////////
//
// Integer Queue
//
/////////////////////////////////////////////////////////////////////

void init_int_queue( Int_queue *q ) {
	q->newest = -1;
	q->oldest = 0;
	q->size = 0;
}

void enqueue_int_queue( int i, Int_queue *q ) {
	bwassert(q->size < INT_QUEUE_SIZE, "enqueue_int_queue: Integer queue should not overflow.");

	q->size++;
	if( ++(q->newest ) >= INT_QUEUE_SIZE) q->newest = 0;
	q->ints[q->newest] = i;
}

int dequeue_int_queue( Int_queue *q ) {
	bwassert(q->size > 0, "enqueue_int_queue: Integer queue should not be empty.");

	q->size--;
	int i = q->ints[q->oldest];
	if(++(q->oldest) >= INT_QUEUE_SIZE) q->oldest = 0;
	return i;
}
