#include <userspace.h>

// ----------------------------------------------------------------------------------------------
// Train Control
// ----------------------------------------------------------------------------------------------
void train_control() {
	
	Create( 10, user_dashboard );

	Create( SWITCHES_SERVER_PRIORITY, switchserver );
	Create( 8, task_cli );

	Exit();
}

