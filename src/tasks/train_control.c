#include <userspace.h>

void train_control() {
	
	Create( 10, user_dashboard );

	Create( SENSOR_SERVER_PRIORITY, sensors_server );
	Create( SWITCHES_SERVER_PRIORITY, switchserver );
	Create( 8, task_cli );

	Exit();
}

