#include <userspace.h>
// -----------------------------------------------------------------------------------------------------------------------------------------------
// Train A Server
// -----------------------------------------------------------------------------------------------------------------------------------------------

void train_a_server() {
	bwdebug( DBG_SYS, "TRAIN_A_SERVER: enters" );
	
	// Data structures
	int sender_tid;
	Reservation_msg reservation_msg;
	
	FOREVER {
		bwdebug( DBG_SYS, "RESERVATION_SERVER: listening for a request" );
		Receive( &sender_tid, ( char * ) &reservation_msg, sizeof( reservation_msg )  );

		switch( reservation_msg.type ){
			//This message can arrive from:
			//	Train A
			//	Train B
			//	Train AI
			case RESERVE_MSG:
				reserve();

				Reply( sender_tid, 0, 0 );
				
				break;

			default:
				bwdebug( DBG_SYS, "ROUTE_SERVER: Invalid request. [type: %d]", route_msg.type );
				break;
		}
	}
}










