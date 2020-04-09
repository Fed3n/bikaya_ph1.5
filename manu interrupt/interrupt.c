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

#ifdef TARGET_UARM
#define INTERRUPT_LINE(cause, line) CAUSE_IP_GET(cause, line)
#define termprint(str) tprint(str);
#endif

#ifdef TARGET_UMPS
#define INTERRUPT_LINE(cause, line) ((cause) & CAUSE_IP(line))
void termprint(char *str);
#endif

extern pcb_t* currentProc;

void stateCpy(state_t* source, state_t* dest){
    #ifdef TARGET_UARM
    dest->a1 = source->a1;    //r0
    dest->a2 = source->a2;    //r1
    dest->a3 = source->a3;    //r2
    dest->a4 = source->a4;    //r3
    dest->v1 = source->v1;    //r4
    dest->v2 = source->v2;    //r5
    dest->v3 = source->v3;    //r6
    dest->v4 = source->v4;    //r7
    dest->v5 = source->v5;    //r8
    dest->v6 = source->v6;    //r9
    dest->sl = source->sl;    //r10
    dest->fp = source->fp;    //r11
    dest->ip = source->ip;    //r12
    dest->sp = source->sp;    //r13
    dest->lr = source->lr;    //r14
    dest->pc = source->pc;    //r15
    dest->cpsr = source->cpsr;
    dest->CP15_Control = source->CP15_Control;
    dest->CP15_EntryHi = source->CP15_EntryHi;
    dest->CP15_Cause = source->CP15_Cause;
    dest->TOD_Hi = source->TOD_Hi;
    dest->TOD_Low = source->TOD_Low;
    #endif

    #ifdef TARGET_UMPS
    dest->entry_hi = source->entry_hi;
    dest->cause = source->cause;
    dest->status = source->status;
    dest->pc_epc = source->pc_epc;
    for(int i = 0; i<30; i++) dest->gpr[i] = source->gpr[i];
    dest->hi = source->hi;
    dest->lo = source->lo;
    #endif
}

void interrupt() {
	termprint("INTERRUPT!\n");
	int line = 0;
	int cause = getCAUSE();
	termprint("Cause got!");
	state_t* p = (state_t *)INT_OLDAREA;
	termprint("a");
	stateCpy(p, &currentProc->p_s);
	termprint("State copied");
	while((line<7) && !(INTERRUPT_LINE(cause, line))) {
		line++;
	}
	if(line == 1) {
		termprint("PLT!\n");
        	updatePriority();
		setTIMER(TIME_SLICE);
		schedule();
	}
	else if(line == 2) {
		termprint("IT!\n");
		updatePriority();
		//setIT(TIME_SLICE)
		schedule();
	}
	else {
		termprint("Cause not implemented yet");
	}
	LDST(TO_LOAD(p));
}



























		
