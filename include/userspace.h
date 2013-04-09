#include "commonspace.h"

// -------------------------------------------------------------------
// User tasks
// -------------------------------------------------------------------

// Communication
#include "tasks/couriers.h"

// Libs
#include "lib/math.h"
#include "lib/train_lost_lib.h"

// Tests
#include "tasks/tests.h"

// Train
#include "tasks/cli.h"
#include "tasks/train_server.h"
#include "tasks/train_control.h"
#include "tasks/train_helpers.h"
#include "tasks/train_movement_data.h"
#include "tasks/train_manager_server.h"
//#include "tasks/train_data_structs.h"
#include "tasks/train_motion.h"
#include "tasks/train_cmd_notifier.h"

// Servers
#include "tasks/location_detection_server.h"
#include "tasks/reservation_server.h"
#include "tasks/route_server.h"
#include "tasks/command_server.h"
#include "tasks/sensors_server.h"
#include "tasks/switches_server.h"

// Track data
#include "tasks/track_helpers.h"
#include "tasks/track_node.h"
#include "tasks/track_data.h"
#include "tasks/track_display.h"

// Others
#include "tasks/calibration_task.h"
#include "tasks/a1.h"
#include "tasks/a3-game.h"
#include "tasks/RPS-game.h"
#include "tasks/user_dashboard.h"
