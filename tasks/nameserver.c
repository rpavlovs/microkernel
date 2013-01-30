#define NAMESERVER_TABLE_MAX_SIZE		200


typedef struct {
	char name[NAMESERVER_MAX_NAME_LENGTH];
	char tid;
} ns_entry;

typedef struct {
	ns_entry entrie[NAMESERVER_TABLE_MAX_SIZE];
	int size;
} ns_table;

init_ns_table( ns_table * table ) {
	table->size = 0;
}

int find_entry( const char * name, const ns_table table ) {
	int pos = 0;
	while( str_cmp( name, table.entrie[pos].name) != 0 && pos < table.size );
	return pos != table.size ? pos : -1; 
}

void nameserver() {

	char msg[ NAMESERVER_MAX_NAME_LENGTH + 1 ];
	char tid;
	int msg_size, pos;
	ns_table table;
	init_ns_table( &table );

	FOREVER {
		msg_size = Recieve( &tid, msg, NAMESERVER_MAX_NAME_LENGTH + 2 );

		switch( msg[0] ) {
		case NAMESERVER_REGISTER_REQUEST:
			pos = find_entry( msg + 1, table );
			if ( pos == -1 ) {
				pos = table.size++;
				mem_cpy( msg+1, table.entrie[pos].name, msg_size - 1);
			}
			table.entrie[pos].tid = tid;
			break;
		case NAMESERVER_WHOIS_REQUEST:
			break;
		}

	}

}