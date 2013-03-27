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
#define DEBUG_LEVEL 						0
//#define DEBUG_AREAS							0 | COMMAND_SERVER_DEBUG_AREA // | UART1_SENDER_DEBUG_AREA // TESTS_DEBUG_AREA | DELAY_DEBUG_AREA
//#define DEBUG_AREAS							0 | TESTS_DEBUG_AREA | DELAY_DEBUG_AREA | KERNEL_DEBUG_AREA // | UART1_SENDER_DEBUG_AREA // TESTS_DEBUG_AREA | DELAY_DEBUG_AREA
//#define DEBUG_AREAS							0x1FFFC0
#define DEBUG_AREAS							0 | TEMP_DEBUG_AREA // TRAIN_MGR_DEBUG_AREA | TRAIN_SRV_DEBUG_AREA //COMMAND_SERVER_DEBUG_AREA | KERNEL_DEBUG_AREA
//#define DEBUG_AREAS							0 | TRAIN_MGR_DEBUG_AREA | TRAIN_SRV_DEBUG_AREA //COMMAND_SERVER_DEBUG_AREA | KERNEL_DEBUG_AREA

#define SYSTEM_DATA_UPDATE_FREQ				2

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
#define COURIER_DEBUG_AREA					1 << 18
#define TRAIN_SRV_DEBUG_AREA				1 << 19
#define TRAIN_MGR_DEBUG_AREA				1 << 20
#define ROUTE_SRV_DEBUG_AREA				1 << 21

// TODO: DELETE THIS DEBUG AREAS
#define DELAY_DEBUG_AREA					1 << 22
#define TEMP_DEBUG_AREA						1 << 23

// ------------------------------------------------------------------------------------------
// Task priorities
// ------------------------------------------------------------------------------------------
#define NAMESERVER_TASK_PRIORITY			10			// NAMESERVER
#define TIMESERVER_TASK_PRIORITY			10			// TIMESERVER
#define CLOCK_TICK_NOTIFIER_PRIORITY		11
#define UART2_SENDER_SERVER_PRIORITY		12			// UART2
#define UART_SENDER_NOTIFIER_PRIORITY		13 
#define UART2_RECEIVER_SERVER_PRIORITY		10
#define UART_RECEIVER_NOTIFIER_PRIORITY		11 
#define UART1_SENDER_SERVER_PRIORITY		10			// UART1
#define UART1_SENDER_NOTIFIER_PRIORITY		11
#define UART1_RECEIVER_SERVER_PRIORITY		14
#define UART1_RECEIVER_NOTIFIER_PRIORITY	15
#define COMMAND_SERVER_PRIORITY				10			// CMD SERVER
#define COMMAND_NOTIFIER_PRIORITY			11
#define SENSOR_SERVER_PRIORITY				12			// SENSOR SERVER
#define SENSOR_NOTIFIER_PRIORITY			13
#define SWITCHES_SERVER_PRIORITY			10			// SWITCHES SERVER
#define CLOCK_TASK_PRIORITY					8			// CLOCK 
#define CLI_PRIORITY						8			// CLI
#define TRAIN_MGR_TASK_PRIORITY				8			// TRAIN MANAGER SERVER
#define TRAIN_TASK_PRIORITY					9			// TRAIN SERVER
#define TRAIN_WAIT_NOT_TASK_PRIORITY		11			// TODO: Modify this to a higher priority
#define TRAIN_SENSOR_NOT_TASK_PRIORITY		11
#define FIRST_TASK_PRIORITY					15			// FIRST TASK
#define FIRST_USER_TASK_PRIORITY			8			// FIRST USER TASK
#define COURIER_TASK_PRIORITY				9			// COURIERS
#define IDLE_TASK_PRIORITY					0			// IDLE TASK.

// ------------------------------------------------------------------------------------------
// Task execution
// ------------------------------------------------------------------------------------------
// #define FIRST_USER_TASK_NAME				run_calibration_constant_velocity train_contol
#define FIRST_USER_TASK_NAME				train_control
// #define FIRST_USER_TASK_NAME				stress_test_uart1_getc
// #define FIRST_USER_TASK_NAME				test_timeserver

#endif
