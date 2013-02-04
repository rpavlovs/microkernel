#include <userspace.h>

#define TIMESERVER_WAKEUP_QUEUE_SIZE 100

typedef struct Wakeup_record Wakeup_record;

struct Wakeup_record {
	int wakeup_time;
	Wakeup_record *next_to_wakeup;
};

typedef struct {
	Wakeup_record tids[TIMESERVER_WAKEUP_QUEUE_SIZE];
	Wakeup_record *first_to_wakeup;
	int size;
} Wakeup_list;

void init_wakeup_list( Wakeup_list *list ) {
	list->first_to_wakeup = 0;
	list->size = 0;
}

void schedule_for_wakeup( Wakeup_list *list, int new_tid, int wakeup_time ) {
	assert( list->size < TIMESERVER_WAKEUP_QUEUE_SIZE,
		"Timeserver wakeup list should not overflow" );
	list->size++;

	Wakeup_record *new_wakeup = &(list->tids[new_tid]);

	if( list->size == 0 || wakeup_time < list->first_to_wakeup->wakeup_time ) {
		new_wakeup->next_to_wakeup = list->first_to_wakeup;
		list->first_to_wakeup = new_wakeup;
		return;
	}

	Wakeup_record *wake_after = list->first_to_wakeup;
	Wakeup_record *wake_before = wake_after->next_to_wakeup;
	
	while( wake_before != 0 && wakeup_time > wake_before->wakeup_time ) {
		wake_after = wake_before;
		wake_before = wake_before->next_to_wakeup;
	}

	wake_after->next_to_wakeup = new_wakeup;
	new_wakeup->next_to_wakeup = wake_before;
}

// Returns:
// 	tid of the process to be waked up
//  -1 - if there is no task to wakeup at the moment
int tid_to_wakeup( Wakeup_list *list, int current_time ) {
	if( list->first_to_wakeup->wakeup_time > current_time ) return -1;

	// Diiference in pointers is a position in an array. It is actually also it's TID
	// I know, it's magic...
	int tid_to_wakeup = (int)(list->first_to_wakeup) - (int)(list->tids);
	assert( tid_to_wakeup >= 0 , "tid_to_wakeup: there is no magic in this world... :(" );

	list->first_to_wakeup = list->first_to_wakeup->next_to_wakeup;	
	list->size--;
	return tid_to_wakeup;
}

void timeserver() {
	debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: START" );
	
	RegisterAs( "timeserver" );
	long current_time = 0;
	int my_tid = MyTid();
	int sender_tid;
	int tid_to_unblock;
	Msg_timeserver_request request;
	Msg_timeserver_reply reply;
	Wakeup_list list;
	init_wakeup_list( &list );

	FOREVER {
		Receive( &sender_tid, (char *) &request, sizeof(request) );
		switch( request.type ) {
		case TICK_NOTIFICATION:
			debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: tick notification recieved from task %d",
				sender_tid );
			current_time++;
			tid_to_unblock = tid_to_wakeup( &list, current_time );
			if( tid_to_unblock >= 0 ) {
				Reply( tid_to_unblock, (char *) &reply, sizeof(reply) );
			}
		case TIME_REQUEST:
			debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: time request recieved from task %d",
				sender_tid );
			reply.type = TIME_REPLY;
			reply.num = current_time;
			Reply( sender_tid, (char *) &reply, sizeof(reply) );
			break;
		case DELAY_REQUEST:
			debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: delay for %d ticks request recieved"
				"from task %d", request.num, sender_tid );
			schedule_for_wakeup( &list, sender_tid, current_time + request.num );
			break;
		case DELAY_UNTIL_REQUEST:
			debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: delay until %d ticks request recieved"
				"from task %d", request.num, sender_tid );
			schedule_for_wakeup( &list, sender_tid, request.num );
			break;
		default:
			debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: *FATAL* unexpected message"
				"[type: %d from: %d]", request.type, sender_tid );
		}
	}

}
