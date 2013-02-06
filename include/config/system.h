#ifndef __BUILD_H
#define __BUILD_H

// Debug levels:
// 0 - panic and assert only
// 1 - user tasks 
// 2 - sytem/user tasks
// 3 - kernel requests + sytem/user tasks
// 4 - all

#define DEBUG_LEVEL 				4

#define NAMESERVER_TASK_PRIORITY	14
#define TIMESERVER_TASK_PRIORITY	14

#define FIRST_USER_TASK_NAME		test_timeserver
#define FIRST_USER_TASK_PRIORITY	8

#endif
