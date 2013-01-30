#include "kernel/kernel_globals.h"
#include "kernel/helpers.h"
#include "kernel/syscall.h"


typedef struct {
	char name[NS_NAME_MAX_LENGTH];
	char tid;
} ns_entry;

typedef struct {
	ns_entry entrie[NS_TABLE_MAX_SIZE];
	int size;
} ns_table;

void init_ns_table( ns_table * table ) {
	table->size = 0;
}

int find_entry( const char * name, const ns_table *table ) {
	int pos = 0;
	while( strcmp( name, table->entrie[pos].name) != 0 && pos < table->size );
	return pos != table->size ? pos : -1; 
}

void nameserver() {

	char msg[ NS_NAME_MAX_LENGTH + 1 ];
	char reply[2];
	int tid;
	int msg_size, pos;
	ns_table table;

	init_ns_table( &table );

	FOREVER {
		msg_size = Receive( &tid, msg, NS_NAME_MAX_LENGTH + 3 );

		switch( msg[0] ) {
		case NS_REQUEST_REGISTER_AS:
			pos = find_entry( msg + 1, &table );
			if ( pos == -1 ) {
				pos = table.size++;
				mem_cpy( msg+1, table.entrie[pos].name, msg_size - 1);
			}
			table.entrie[pos].tid = tid;
			reply[0] = SUCCESS;
			break;
		
		case NS_REQUEST_WHO_IS:

			pos = find_entry( msg + 1, &table );
			if( pos == -1 ) {
				reply[0] = ( -NS_ERROR_TASK_NOT_FOUND );
			} else {
				reply[0] = SUCCESS;
				reply[1] = table.entrie[pos].tid;
			}
			break;
		}
		Reply( tid, reply, 2 );
	}
}
