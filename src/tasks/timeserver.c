#include <userspace.h>

#define TIMESERVER_WAKEUP_QUEUE_SIZE 100

// ----------------------------------------------------------------------------
// Timer
// ----------------------------------------------------------------------------
void start_timer(){
	int timerControlValue; 
	int *timerLoad = ( int * ) TIMER1_BASE;
	int *timerControl = ( int * ) ( TIMER1_BASE + CRTL_OFFSET ); 

	// First the load is added. 
	*timerLoad = INITIAL_TIMER_LOAD;

	// The timer is enabled and configured.
	timerControlValue = *timerControl;
	timerControlValue = timerControlValue | TIMER_ENABLE_FLAG | TIMER_MODE;
	*timerControl = timerControlValue;
}

// ----------------------------------------------------------------------------
// Time Server
// ----------------------------------------------------------------------------
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
	new_wakeup->wakeup_time = wakeup_time;

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
	debug( DBG_SYS, "TIMESERVER: task %d is scheduled to run after %d ticks",
			new_tid, new_wakeup->wakeup_time );
}

// Note: Returns -1 if there is no task to wakeup at the moment
int get_tid_to_wakeup( Wakeup_list *list, int current_time ) {
	if( list->size == 0 || list->first_to_wakeup->wakeup_time > current_time )
		return -1;

	// Diiference in pointers is a position in an array. It is actually also it's TID
	// It's magic, I know...
	
	debug( DBG_SYS, "TIMESERVER: list->tids = %d, list->first_to_wakeup = %d ",
			list->tids, list->first_to_wakeup );

	int tid_to_wakeup = (int)(list->tids - list->first_to_wakeup);
	assert( tid_to_wakeup >= 0 , "tid_to_wakeup: there is no magic in this world... :(" );

	list->first_to_wakeup = list->first_to_wakeup->next_to_wakeup;	
	list->size--;
	return tid_to_wakeup;
}

void clock_tick_notifier() {
	debug( DBG_SYS, "TICK_NOTIFIER: start with tid %d", MyTid() );
	int timeserver_tid = WhoIs( "timeserver" );
	Msg_timeserver_request msg;
	msg.type = TICK_NOTIFICATION;

	FOREVER {
		// AwaitEvent( );
		debug( DBG_SYS, "TICK_NOTIFIER: sending a tick to %d",
			timeserver_tid);
		Send( timeserver_tid, (char *) &msg, sizeof(msg), (char *) 0, 0 );
	}
}

void timeserver() {
	debug( DBG_SYS, "TIMESERVER: start with tid %d", MyTid() );
	
	start_timer(); 
	
	RegisterAs( "timeserver" );
	long current_time = 0;
	int sender_tid;
	int tid_to_unblock;
	Msg_timeserver_request request;
	Msg_timeserver_reply reply;
	Wakeup_list list;
	init_wakeup_list( &list );
	Create( 4, clock_tick_notifier );

	FOREVER {
		Receive( &sender_tid, (char *) &request, sizeof(request) );
		switch( request.type ) {
		case TICK_NOTIFICATION:
			debug( DBG_SYS, "TIMESERVER: tick notification recieved from task %d",
				sender_tid );
			current_time++;
			tid_to_unblock = get_tid_to_wakeup( &list, current_time );
			if( tid_to_unblock >= 0 ) {
				debug( DBG_SYS, "TIMESERVER: unblocking task %d",
					tid_to_unblock );
				Reply( tid_to_unblock, (char *) 0, 0 );
			}
			debug( DBG_SYS, "TIMESERVER: tick notification reply dispatched" );
			Reply( sender_tid, (char *) 0, 0 );
			break;
		case TIME_REQUEST:
			debug( DBG_SYS, "TIMESERVER: time request recieved from task %d",
				sender_tid );
			reply.type = TIME_REPLY;
			reply.num = current_time;
			Reply( sender_tid, (char *) &reply, sizeof(reply) );
			break;
		case DELAY_REQUEST:
			debug( DBG_SYS, "TIMESERVER: delay for %d ticks request recieved "
				"from task %d", request.num, sender_tid );
			schedule_for_wakeup( &list, sender_tid, current_time + request.num );
			break;
		case DELAY_UNTIL_REQUEST:
			debug( DBG_SYS, "TIMESERVER: delay until %d ticks request recieved "
				"from task %d", request.num, sender_tid );
			schedule_for_wakeup( &list, sender_tid, request.num );
			break;
		default:
			debug( DBG_SYS, "TIMESERVER: *FATAL* unexpected message "
				"[type: %d from: %d]", request.type, sender_tid );
		}
	}

}
