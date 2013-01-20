//First byte of RAM memory
#define RAM_START		0x00000000

//RedBoot memory (Default value. Around 2MB)
#define REDBOOT_START		0x00000000
#define REDBOOT_END		0x00217FFF

//ELF file memory (Around 2MB)
#define ELF_START		0x00218000
#define ELF_END			0x003FFFFFK

//Leftovers of RAM (Around 0MB)
#define FREE_MEMORY_END		0x00400000
#define FREE_MEMORY_START	0x01DE6FFF

//Tasks' stacks memory
#define TASKS_MEMORY_END	0x0143C000??????
#define TASKS_MEMORY_START	0x01DFFFFF

//First task's stack memory
#define FIRST_TASK_END		0x01DE7000
#define FIRST_TASK_START	0x01DFFFFF

//Tasks' data structures sizes
#define MAX_TASKS_COUNT		100
#define TASK_DESCRIPTOR_SIZE	100		//100 B
#define TASK_STACK_SIZE		102400		//100 kB
#define TASKS_STACKS_SIZE	10240000	//100 kB * 100; 0x009C4000 B

//Kernel stack memory. 2MB are allocated
#define KERNEL_STACK_END	0x01E00000
#define KERNEL_STACK_START	0x01FFFFFF

//Last address of RAM memory
#define RAM_END			0x01FFFFFF

//First address of ROM memory
#define ROM_START		0x60000000
