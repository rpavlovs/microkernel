


void interrupt_handle( ) {
	// get here immidiately after swi was called

	// Switch to system mode ( spack pointer becomes the same as in the user mode)
	// Save r0-r12 on the task's stack
	// Switch back to supervisor mode
	// 
	// get pc from the task's stack an put it in supervisor's link register (lr_svc)
}