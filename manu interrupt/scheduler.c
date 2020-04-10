#include "pcb.h"
#include "const.h"
#include "scheduler.h"

#ifdef TARGET_UMPS
#include "libumps.h"
#include "arch.h"
#include "cp0.h"
#include "exc_area.h"
#endif

#ifdef TARGET_UARM
#include "libuarm.h"
#include "arch.h"
#endif

#include "const.h"
#include "pcb.h"
#include "types_bikaya.h"
#include "auxfun.h"
#include "interrupt.h"
#include "scheduler.h"

#ifdef TARGET_UMPS
void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

HIDDEN LIST_HEAD(readyQueue_h);
//SPOSTATO IN .H -->( pcb_t* currentProc; )

/*AREA MOLTO SPERIMENTALE*/
/*stato vuoto caricato se la ready queue è vuota in attesa di un nuovo processo*/
state_t waitingState;

void initProcess_KM(state_t* p, void* fun, int n);

static void wait4proc(){
	termprint("Now waiting\n");
	for(;;)
		;
}

/*************************/

void initReadyQueue(){
	mkEmptyProcQ(&readyQueue_h);
	initProcess_KM(&waitingState, wait4proc, 1);
}

int emptyReadyQueue(){
	return emptyProcQ(&readyQueue_h);
}

void insertReadyQueue(pcb_t* proc){
	insertProcQ(&readyQueue_h, proc);
}

pcb_t* removeReadyQueue(){
	return removeProcQ(&readyQueue_h);
}

void remove2ReadyQueue(pcb_t* proc){
	pcb_t* p = outProcQ(&readyQueue_h,proc);
}

pcb_t* headReadyQueue(){
	return headProcQ(&readyQueue_h);
}

void terminateProc(){
	if(currentProc != NULL){
		freePcb(currentProc);
		currentProc = NULL;
	}
}

void terminateProc2(pcb_t* proc){
	if(proc != NULL){
		freePcb(proc);
		proc = NULL;
	}
}

void updatePriority(){
	if(currentProc != NULL){
		/*reset alla priorità del processo in controllo*/
		currentProc->priority = currentProc->original_priority;
	}
	/*update alla priorità di tutti i processi in ready queue*/
	pcb_t* p;
	list_for_each_entry(p, &readyQueue_h, p_next){
		p->priority = p->priority + AGING_AMOUNT;
	}
}

void schedule(){
	termprint("SCHEDULE!\n");

	/*Processo in esecuzione dovrebbe venire così messo primo in coda
	se non ci sono state modifiche alla priorità, a meno che non sia
	stato terminato*/
	if(currentProc != NULL){
		insertReadyQueue(currentProc);
	}
	setTIMER(3000);
	/*Se non ci sono processi da schedulare, lo scheduler attende*/
	if(emptyReadyQueue())
		LDST(TO_LOAD((&waitingState)));
	
	currentProc = removeReadyQueue();
	state_t* p = &(currentProc->p_s);
	LDST(TO_LOAD(p));
}
