#include "userspace.h"

int find_entry( const char * name, const ns_table *table ) {
	int pos = -1;
	while( strcmp( name, table->entrie[++pos].name ) != 0 && pos < table->size );
	return pos != table->size ? pos : -1; 
}

void nameserver() {
	bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: enters" );

	Nameserver_request request;
	Nameserver_reply reply;

	int sender_tid;
	int pos;

	ns_table table;
	table.size = 0;

	FOREVER {
		bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: listening for a request" );
		Receive( &sender_tid, (char *) &request, sizeof(request) );

		switch( request.type ) {
		case NAMESERVER_REGISTER_AS_REQUEST:
			bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: RegisterAs request recived. [ sender: %d name: %s]", sender_tid, request.ns_name );
			pos = find_entry( request.ns_name, &table );
			if ( pos == -1 ) {
				bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: new ns record required" );
				pos = table.size++;
				strcpy( request.ns_name, table.entrie[pos].name );
			}
			table.entrie[pos].tid = sender_tid;
			reply.num = SUCCESS;

			break;
		case NAMESERVER_WHO_IS_REQUEST:
			bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: WhoIs request recived from %d", sender_tid );
			pos = find_entry( request.ns_name, &table );
			reply.num = (pos < 0 ? NS_ERROR_TASK_NOT_FOUND : table.entrie[pos].tid);
			break;
		default:
			bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: Nameserver received unexpected "
				"message [type: %d from: %d]", request.type, sender_tid );
			reply.num = ERROR_WRONG_MESSAGE_TYPE;
		}

		if( reply.num < 0 )
			bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: Could not find [%s] requested "
				"by task %d", request.ns_name, sender_tid );
		
		bwdebug( DBG_SYS, NAMESERVER_DEBUG_AREA, "NAMESERVER: replying to: %d [msg: %d]",
			sender_tid, reply.num );
		Reply( sender_tid, (char *) &reply, sizeof(reply) );
	}
}
