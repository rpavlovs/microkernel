
void initialize(){
	//Where is the kernel entry?
	//Turn off interrupts in ICU!
	//Prepare the kernel data structures
		//Where is the kernel stack pointer right now? (Kernel stack pointer is in the end of the RAM memory)
		//What does the stack look like? (Stack goes downwards from the end of the RAM memory)
		//Do we want it there? YES
		//Do we want it somewhere else? NO
		//This is the last chance to change it! OK :)
		//If we are going to change it - it is a good idea to keep changed stuff around! WHATEVER :)
	//Kernel data structures
		//An array of empty ready queues
		//A pointer to the TD of the active task
		//An array of TDs
		//A free list of pointers to free TDs
	//Prepare memory to be used by tasks
		//Allocate task memory
	//Create first user task
		//Can run with interrupts turned off by now
	//NOTE: the place where the kernel starts executing has the global name main which cannot be reused



//Where is the kernel stack pointer right now?
__asm__(
	"mov r0, #1\n"
	"mov r1, sp"
	"bl bwputr\n"
);

}
