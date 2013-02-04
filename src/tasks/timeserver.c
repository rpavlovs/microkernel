#include <userspace.h>


void timeserver() {
	debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: START" );
	long current_time = 0;
	int my_tid = MyTid();
	int sender_tid;
	Msg_timeserver_request incoming_msg;
	Msg_timeserver_reply reply_msg;

	RegisterAs( "timeserver" );

	FOREVER {
		Receive( &sender_tid, (char *) &incoming_msg, sizeof(incoming_msg) );
		switch( incoming_msg.type ) {
		case TIME_REQUEST_MESSAGE:
			debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: time request recieved from task %d",
				sender_tid );
			reply_msg.type = TIME_REPLY_MESSAGE;
			reply_msg.num = current_time;
			Reply( sender_tid, (char *) &reply_msg, sizeof(reply_msg) );
			break;
		default:
			debug( DBG_CURR_LVL, DBG_SYS, "TIMESERVER: *FATAL* unexpected message"
				"[type: %d from: %d]", incoming_msg.type, sender_tid );
		}
	}

}
