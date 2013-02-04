#include "userspace.h"

int find_entry( const char * name, const ns_table *table ) {
	int pos = -1;
	while( strcmp( name, table->entrie[++pos].name ) != 0 && pos < table->size );
	return pos != table->size ? pos : -1; 
}

void nameserver() {
	debug( DBG_CURR_LVL, DBG_SYS, "NAMESERVER: start" );

	Nameserver_request request;
	Nameserver_reply reply;

	int sender_tid;
	int pos;

	ns_table table;
	table.size = 0;

	FOREVER {
		debug( DBG_CURR_LVL, DBG_SYS, "NAMESERVER: Block to recieve request" );
		Receive( &sender_tid, (char *) &request, sizeof(request) );

		switch( request.type ) {
		case NAMESERVER_REGISTER_AS_REQUEST:
			debug( DBG_CURR_LVL, DBG_SYS, "NAMESERVER: RegisterAs request recived" );
			pos = find_entry( request.ns_name, &table );
			if ( pos == -1 ) {
				debug( DBG_CURR_LVL, DBG_SYS, "NAMESERVER: new ns record required" );
				pos = table.size++;
				my_strcpy( request.ns_name, table.entrie[pos].name );
			}
			table.entrie[pos].tid = sender_tid;
			reply.num = SUCCESS;
			break;
		case NAMESERVER_WHO_IS_REQUEST:
			debug( DBG_CURR_LVL, DBG_SYS, "NAMESERVER: WhoIs request recived" );
			pos = find_entry( request.ns_name, &table );
			reply.num = (pos >= 0 ? table.entrie[pos].tid : NS_ERROR_TASK_NOT_FOUND);
			break;
		default:
			debug( DBG_CURR_LVL, DBG_SYS, "**FATAL** NAMESERVER: Nameserver received"
				"message of type %d", request.type );
			reply.num = ERROR_WRONG_MESSAGE_TYPE;
		}
		
		Reply( sender_tid, (char *) &reply, sizeof(reply) );
	}
}