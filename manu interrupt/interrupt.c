#include "interrupt.h"
#include "const.h"

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

void interrupt_handler() {
	extern pcb_t* currentProc;
	termprint("INTERRUPT!\n");
	/*prima di ridare controllo al processo qua dovremmo diminuire di 1 word il pc a uarm, niente su umps*/
	int line = 0;
	while(line<=7 && !(INTERRUPT_LINE_CAUSE(getCAUSE(), line))) line++;
	/*Siccome il PLT non e’ presente su uARM, e’
	conveniente sfruttare l’interval timer su
	entrambe le piattaforme*/
	state_t* p = &currentProc->p_s;
	p = (state_t*)INT_OLDAREA;
	memcpy((state_t*) INT_OLDAREA, &currentProc->p_s, sizeof(state_t*));
	switch(line){
		case PROCESSOR_LOCAL_TIMER:
			termprint("PLT!\n");
			break;
		case BUS_INTERVAL_TIMER:
			termprint("IT!\n");
			updatePriority();
			termprint("Aggiornate le priority");
			setTIMER(TIME_SLICE);
			schedule();
		default:
			termprint("Linea non ancora implementata\n");
			HALT();
	}
}
