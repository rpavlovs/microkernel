#include "userspace.h"

int is_train_id( int id ) {
	int TRAIN_IDS[NUM_TRAINS_SUPPORTED] = LIST_TRAINS_SUPPORTED;
	int i;
	for( i = 0; i < NUM_TRAINS_SUPPORTED; ++i ) {
		if( TRAIN_IDS[i] == id ) return 1;
	}
	return 0;
}
