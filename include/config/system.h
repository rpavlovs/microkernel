#ifndef __BUILD_H
#define __BUILD_H

// Debug levels:
// 0 - panic and assert only
// 1 - user tasks 
// 2 - sytem/user tasks
// 3 - kernel requests + sytem/user tasks
// 4 - all
// 
// Note: requests with non-zero nubmer of arguments
//  do not have debug printouts


#define DEBUG_LEVEL 					0

#define NAMESERVER_TASK_PRIORITY			12
#define TIMESERVER_TASK_PRIORITY			12
#define UART2_SENDER_SERVER_PRIORITY		14
#define UART2_RECEIVER_SERVER_PRIORITY	12


#define FIRST_USER_TASK_NAME				train_control
#define FIRST_USER_TASK_PRIORITY			8

#endif
