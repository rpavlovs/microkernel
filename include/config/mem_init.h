//First byte of RAM memory
#define RAM_START		0x00000000

//RedBoot memory (Default value. Approx. 2MB)
#define REDBOOT_START		0x00000000
#define REDBOOT_END		0x00217FFF

//ELF file memory (Approx. 2MB)
#define ELF_START		0x00218000
#define ELF_END			0x003FFFFF

//Leftovers of RAM (Approx. 16MB)
#define FREE_MEMORY_END		0x00400000
#define FREE_MEMORY_START	0x0143BFFF

//Tasks' stacks memory
#define TASKS_MEMORY_END	0x0143C000
#define TASKS_MEMORY_START	0x01DFFFFF
#define TASKS_MEMORY_PER_INSTANCE	( TASKS_MEMORY_START - TASKS_MEMORY_END ) / MAX_NUM_TASKS

//First task's stack memory
#define FIRST_TASK_END		0x01DE7000
#define FIRST_TASK_START	0x01DFFFFF

//Tasks' data structures sizes
#define MAX_TASKS_COUNT		100
#define TASK_DESCRIPTOR_SIZE	128		//128 B
