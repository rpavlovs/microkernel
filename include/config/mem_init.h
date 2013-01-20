//This is the first byte of RAM memory
#define RAM_START		0x00000000

//
#define REDBOOT_START		0x00000000
#define REDBOOT_END		0x00217FFF

#define ELF_START		0x00218000
#define ELF_END			0x003FFFFF

#define TASKS_MEMORY_END	0x00400000
#define TASKS_MEMORY_START	0x01DFFFFF

#define FIRST_TASK_END		
#define FIRST_TASK_START	0x01DFFFFF

#define MAX_TASKS_COUNT		100
#define TASK_DESCRIPTOR_SIZE	100
#define TASK_STACK_SIZE		102400

//For the kernel stack 2MB of memory is allocated
#define KERNEL_STACK_END	0x01E00000
#define KERNEL_STACK_START	0x01FFFFFF

//This is the last byte of 32MB
#define RAM_END			0x01FFFFFF

//This is the first byte of ROM memory
#define ROM_START		0x60000000
