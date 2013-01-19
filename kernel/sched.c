

int activate( active ) {
	return 0;
}


int schedule( Schedule *sched ) {
	return 0;
}

int getNextRequest( ) {
  return activate( schedule( ) ); //the active task doesn't change
}

// Initialize Schedule sturct
void init_schedule( Schedule *sched ) {
	int i;
	for (i = 0; i < SCHED_NUM_PRIORITIES; ++i)
	{
		sched->priority[i]->oldest = 0;
		sched->priority[i]->newest = 0;
		sched->priority[i]->size = 0;
	}
	sched->latest_tid = 0;
}

// Add process to scheduler
int Create( int priority, void (*code) ( ), Schedule *sched ) {
	
	if( sched->priority[priority]->size > SCHED_QUEUE_MAX_LENGTH ) {
		bwprintf( COM2, "ERROR: Scheduler round buffer overflow.");
		return -3;
	}

	if( sched->platest_tid >= SCHED_TID_MAX_VAL ) {
		bwprintf( COM2, "ERROR: Scheduler is out of task descriptors.");
		return -2;
	}
	
	if( priority < 0 || priority >= SCHED_NUM_PRIORITIES ) {
		bwprintf( COM2, "ERROR: Scheduler was given a wrong task priority.");
		return -1;
	}
	
	if( sched->priority[priority]->size == 0 ) {
		Process *new_proc = sched->priority[priority]->buffer;
		sched->priority[priority]->newest = new_proc;
		sched->priority[priority]->oldest = new_proc;
		sched->priority[priority]->size = 1;
		new_proc->tid = sched->latest_tid++;
		return new_proc->tid;
	}

	Process *new_proc = sched->priority[priority]->newest + 1;
	if( new_proc >= sched->priority[priority]->buffer + SCHED_QUEUE_MAX_LENGTH ) {
		new_proc = sched->priority[priority]->buffer;
	}
	
	sched->priority[priority]->newest = new_proc;
	sched->priority[priority]->size++;
	new_proc->tid = sched->latest_tid++;
	return new_proc->tid;

}