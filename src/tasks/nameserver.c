#include "userspace.h"


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
	int pos = -1;
	while( strcmp( name, table->entrie[++pos].name ) != 0 && pos < table->size );
	return pos != table->size ? pos : -1; 
}

void nameserver() {

	debug( "starting nameserver" );

	char msg[ NS_NAME_MAX_LENGTH + 1 ];
	char reply[2];
	int sender_tid;
	int msg_size, pos;
	ns_table table;

	init_ns_table( &table );

	FOREVER {

		debug( "Nameserver: Recieving request" );
		
		msg_size = Receive( &sender_tid, msg, NS_NAME_MAX_LENGTH + 3 );

		// bwprintf( COM2, "DEBUG: Nameserver: Recieved new request. TID: %d MSG: [%d][%s]\n",
		// 	sender_tid, msg[0], msg + 1 );

		switch( msg[0] ) {
		case NS_REQUEST_REGISTER_AS:
			debug( "nameserver: RegisterAs request recived" );
			pos = find_entry( msg + 1, &table );
			if ( pos == -1 ) {
				debug( "nameserver: new ns record required" );
				pos = table.size++;
				my_strcpy( msg + 1, table.entrie[pos].name );
			}
			table.entrie[pos].tid = sender_tid;
			reply[0] = SUCCESS;
			break;
		
		case NS_REQUEST_WHO_IS:
			debug( "nameserver: WhoIs request recived" );
			pos = find_entry( msg + 1, &table );
			if( pos == -1 ) {
				reply[0] = ( -NS_ERROR_TASK_NOT_FOUND );
			} else {
				reply[0] = SUCCESS;
				reply[1] = table.entrie[pos].tid;
			}
			break;
		}

		// bwprintf( COM2, "DEBUG: Nameserver: reply to TID: %d reply[0]: %d reply[1]: [%d]\n",
		// 	sender_tid, reply[0], reply[1] );
		
		Reply( sender_tid, reply, 2 );
	}
}
