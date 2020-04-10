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

/*inizializza l'exception area in kernel mode, interrupt disabilitati*/
void init_excarea(state_t* p, void* handler){
	ownmemset(p, 0, sizeof(state_t));
	p->reg_sp = RAMTOP;
	/*Inizializzo sia pc_epc che reg_t9 all'entry point come dal manuale di umps*/
	p->pc_epc = (memaddr)handler;
	p->reg_t9 = (memaddr)handler;
	p->status = STATUS_ALL_INT_DISABLE_KM_LT(p->status);
}

/*inizializza processo in kernel mode, interrupt disabilitati escluso timer*/
/*n = numero del processo, che influenza dove gli viene assegnato spazio in ram*/
void initProcess_KM(state_t* p, void* fun, int n){
	ownmemset(p, 0, sizeof(state_t));
	p->reg_sp = (RAMTOP-(RAM_FRAMESIZE*n));
	/*Inizializzo sia pc_epc che reg_t9 all'entry point come dal manuale di umps*/
	p->pc_epc = (memaddr)fun;
	p->reg_t9 = (memaddr)fun;
	p->status = STATUS_ALL_INT_ENABLE_KM_LT(p->status);
}

#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);

/*inizializza l'exception area in kernel mode, interrupt disabilitati*/
void init_excarea(state_t* p, void* handler){
	ownmemset(p, 0, sizeof(state_t));
	p->pc = (memaddr)handler;
	p->sp = RAMTOP;
	p->cpsr = (p->cpsr | STATUS_SYS_MODE);
	p->cpsr = STATUS_ALL_INT_DISABLE(p->cpsr);
	p->CP15_Control = CP15_DISABLE_VM(p->CP15_Control);
}

/*inizializza processo in kernel mode, interrupt disabilitati escluso timer*/
/*n = numero del processo, che influenza dove gli viene assegnato spazio in ram*/
void initProcess_KM(state_t* p, void* fun, int n){
	ownmemset(p, 0, sizeof(state_t));
	p->pc = (memaddr)fun;
	p->sp = (RAMTOP-(RAM_FRAMESIZE*n));
	p->cpsr = (p->cpsr | STATUS_SYS_MODE);
	p->cpsr = STATUS_DISABLE_INT(p->cpsr);
	p->cpsr = STATUS_ENABLE_TIMER(p->cpsr);
	p->CP15_Control = CP15_DISABLE_VM(p->CP15_Control);
}

#endif


extern void interrupt();

void test1();
void test2();
void test3();

void testx(){
	for(;;)
		;
}

void test(){
	termprint("Hi there!\n");
	SYSCALL(0,0,0,0);
	termprint("Back to test\n");
}

void handleINT(){
	extern pcb_t* currentProc;
	termprint("INTERRUPT!\n");
	/*prima di ridare controllo al processo qua dovremmo diminuire di 1 word il pc a uarm, niente su umps*/
	int line = 0;
	while(line<=7 && !(INTERRUPT_LINE_CAUSE(getCAUSE(), line))) line++;
	/*Siccome il PLT non e’ presente su uARM, e’
	conveniente sfruttare l’interval timer su
	entrambe le piattaforme*/
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

void handleTLB(){
	termprint("TLB!");
	HALT();
}

void handleTRAP(){
	termprint("TRAP!");
	HALT();
}

void handleSYSBP(){
	termprint("SYSBP!");
	terminateProc();
	schedule();
	state_t* p = (state_t *)SYSBK_OLDAREA;
	/*prima di ridare controllo al processo incrementiamo di 1 word il pc a umps, niente su uarm*/
	p->ST_PC = p->ST_PC + SYSBP_PC*WORDSIZE;
	LDST(TO_LOAD(p));
	HALT();
}

/*Prima inizializzo tutte le aree a 0, poi assegno i campi richiesti con una macro*/
void initAreas(){
/*AREA INTERRUPT*/
init_excarea((state_t *)INT_NEWAREA, handleINT);

/*AREA TLB*/
init_excarea((state_t *)TLB_NEWAREA, handleTLB);

/*AREA PGMTRAP*/
init_excarea((state_t *)PGMTRAP_NEWAREA, handleTRAP);

/*AREA SYSKB*/
init_excarea((state_t *)SYSBK_NEWAREA, handleSYSBP);

}

/*Inizializza le exception area, i PCBs e poi avvia un processo con LDST*/
int main(){
	initAreas();
	termprint("AREA DONE!\n");

	initPcbs();
	termprint("PCB DONE!\n");

	struct pcb_t* a = allocPcb();
	initProcess_KM(&a->p_s, test, 1);
	termprint("PROCESS INITIALIZED!\n");

	/*Qua nella versione finale immagino andrà chiamato schedule() (e forse prima inizializzato il timer)*/
	/*
	state_t* p = &(a->p_s);
	LDST(TO_LOAD(p));
	*/
	initReadyQueue();
	insertReadyQueue(a);
	schedule();
	/*Se arriva qua sotto dopo LDST qualcosa è andato così storto dall'aver infranto ogni regola dell'emulatore*/
	termprint("Oh no\n");

	for(;;)
		;
}
