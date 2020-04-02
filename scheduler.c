#include "pcb.h"
#include "scheduler.h"

HIDDEN LIST_HEAD(readyQueue_h);

void initReadyQueue(){
	mkEmptyProcQ(&readyQueue_h);
}

void insertReadyQueue(struct pcb_t* proc){
	insertProcQ(&readyQueue_h, &proc);
}

pcb_t* removeReadyQueue(){
	return removeProcQ(&readyQueue_h)
}

pcb_t* headReadyQueue(){
	return headProcQ(&readyQueue_h);
}

void updatePriority(){
	termprint(":)");
}

void schedule(int action){
	termprint("SCHEDULE!\n");

	if(action == UPDATE_PRIORITY){
		updatePriority();
	}

	struct pcb_t* proc = headReadyQueue();

	LDST(&proc->s_t);
}