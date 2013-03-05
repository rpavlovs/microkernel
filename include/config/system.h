#ifndef __BUILD_H__
#define __BUILD_H__

// Debug levels:
// 0 - panic and bwassert only
// 1 - user tasks 
// 2 - sytem/user tasks
// 3 - kernel requests + sytem/user tasks
// 4 - all


#define DEBUG_LEVEL 						0

#define NAMESERVER_TASK_PRIORITY			12
#define TIMESERVER_TASK_PRIORITY			12
#define UART2_SENDER_SERVER_PRIORITY		14
#define UART2_RECEIVER_SERVER_PRIORITY		12
#define UART1_SENDER_SERVER_PRIORITY		14
#define UART1_RECEIVER_SERVER_PRIORITY		12


#define FIRST_USER_TASK_NAME				train_control
// #define FIRST_USER_TASK_NAME				stress_test_uart1_getc
#define FIRST_USER_TASK_PRIORITY			8

#endif
