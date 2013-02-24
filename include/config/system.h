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


#define DEBUG_LEVEL 				1

#define NAMESERVER_TASK_PRIORITY	14
#define TIMESERVER_TASK_PRIORITY	14

#define FIRST_USER_TASK_NAME		task_test_uart1
#define FIRST_USER_TASK_PRIORITY	14

#endif
