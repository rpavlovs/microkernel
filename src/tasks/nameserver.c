#include "userspace.h"

int find_entry( const char * name, const ns_table *table ) {
	int pos = -1;
	while( strcmp( name, table->entrie[++pos].name ) != 0 && pos < table->size );
	return pos != table->size ? pos : -1; 
}

void nameserver() {
	debug( DBG_SYS, "NAMESERVER: enters" );

	Nameserver_request request;
	Nameserver_reply reply;

	int sender_tid;
	int pos;

	ns_table table;
	table.size = 0;

	FOREVER {
		debug( DBG_SYS, "NAMESERVER: listening for a request" );
		Receive( &sender_tid, (char *) &request, sizeof(request) );

		switch( request.type ) {
		case NAMESERVER_REGISTER_AS_REQUEST:
			debug( DBG_SYS, "NAMESERVER: RegisterAs request recived from %d", sender_tid );
			pos = find_entry( request.ns_name, &table );
			if ( pos == -1 ) {
				debug( DBG_SYS, "NAMESERVER: new ns record required" );
				pos = table.size++;
				strcpy( request.ns_name, table.entrie[pos].name );
			}
			table.entrie[pos].tid = sender_tid;
			reply.num = SUCCESS;
			break;
		case NAMESERVER_WHO_IS_REQUEST:
			debug( DBG_SYS, "NAMESERVER: WhoIs request recived from %d", sender_tid );
			pos = find_entry( request.ns_name, &table );
			reply.num = (pos < 0 ? NS_ERROR_TASK_NOT_FOUND : table.entrie[pos].tid);
			break;
		default:
			debug( DBG_SYS, "NAMESERVER: Nameserver received unexpected "
				"message [type: %d from: %d]", request.type, sender_tid );
			reply.num = ERROR_WRONG_MESSAGE_TYPE;
		}

		if( reply.num < 0 )
			debug( DBG_SYS, "NAMESERVER: Could not find [%s] requested "
				"by task %d", request.ns_name, sender_tid );
		
		debug( DBG_SYS, "NAMESERVER: replying to: %d [msg: %d]",
			sender_tid, reply.num );
		Reply( sender_tid, (char *) &reply, sizeof(reply) );
	}
}
