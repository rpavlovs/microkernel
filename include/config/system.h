#ifndef __BUILD_H__
#define __BUILD_H__

// ------------------------------------------------------------------------------------------
// DEBUG
// ------------------------------------------------------------------------------------------
// Debug levels:
// 0 - panic and bwassert only
// 1 - user tasks 
// 2 - sytem/user tasks
// 3 - kernel requests + sytem/user tasks
// 4 - all
#define DEBUG_LEVEL 						1
#define DEBUG_AREAS							0 | SENSORS_SERVER_DEBUG_AREA | SWITCHES_SERVER_DEBUG_AREA | COMMAND_SERVER_DEBUG_AREA

// Do this in an enum? 
#define KERNEL_DEBUG_AREA					1 << 0
#define SCHEDULER_DEBUG_AREA				1 << 1
#define HWI_DEBUG_AREA						1 << 2
#define SWI_DEBUG_AREA						1 << 3
#define CONTEXT_SW_DEBUG_AREA				1 << 4
#define TIMESERVER_DEBUG_AREA				1 << 5
#define NAMESERVER_DEBUG_AREA				1 << 6
#define UART1_SENDER_DEBUG_AREA				1 << 7
#define UART1_RECEIVER_DEBUG_AREA			1 << 8
#define UART2_SENDER_DEBUG_AREA				1 << 9
#define UART2_RECEIVER_DEBUG_AREA			1 << 10
#define COMMAND_SERVER_DEBUG_AREA			1 << 11
#define SWITCHES_SERVER_DEBUG_AREA			1 << 12
#define SENSORS_SERVER_DEBUG_AREA			1 << 13
#define CLI_DEBUG_AREA						1 << 14
#define IDLE_TASK_DEBUG_AREA				1 << 15
#define TESTS_DEBUG_AREA					1 << 16
#define RPS_GAME_DEBUG_AREA					1 << 17

// ------------------------------------------------------------------------------------------
// Task priorities
// ------------------------------------------------------------------------------------------
#define FIRST_TASK_PRIORITY					15
#define IDLE_TASK_PRIORITY					0

#define CLOCK_TASK_PRIORITY					8
#define NAMESERVER_TASK_PRIORITY			12
#define TIMESERVER_TASK_PRIORITY			12
#define UART2_SENDER_SERVER_PRIORITY		14
#define UART2_RECEIVER_SERVER_PRIORITY		14
#define UART1_SENDER_SERVER_PRIORITY		14
#define UART1_RECEIVER_SERVER_PRIORITY		14

// ------------------------------------------------------------------------------------------
// Task execution
// ------------------------------------------------------------------------------------------
// #define FIRST_USER_TASK_NAME				run_calibration_constant_velocity
#define FIRST_USER_TASK_NAME				train_control
//#define FIRST_USER_TASK_NAME				stress_test_uart1_getc
#define FIRST_USER_TASK_PRIORITY			8

#endif
