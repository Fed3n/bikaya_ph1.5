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
#include "scheduler.h"
#include "interrupt.h"
#include "handler.h"

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
void initProcess_KM(pcb_t* p, void* fun, int n){
	p->p_s.reg_sp = (RAMTOP-(RAM_FRAMESIZE*n));
	/*Inizializzo sia pc_epc che reg_t9 all'entry point come dal manuale di umps*/
	p->p_s.pc_epc = (memaddr)fun;
	p->p_s.reg_t9 = (memaddr)fun;
	p->p_s.status = STATUS_ALL_INT_ENABLE_KM_LT(p->p_s.status);
	p->original_priority = n;
	p->priority = n;
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
/*n = numero del processo, che influenza la priorità e dove gli viene assegnato spazio in ram*/
void initProcess_KM(pcb_t* p, void* fun, int n){
	p->p_s.pc = (memaddr)fun;
	p->p_s.sp = (RAMTOP-(RAM_FRAMESIZE*n));
	p->p_s.cpsr = (p->p_s.cpsr | STATUS_SYS_MODE);
	p->p_s.cpsr = STATUS_DISABLE_INT(p->p_s.cpsr);
	p->p_s.cpsr = STATUS_ENABLE_TIMER(p->p_s.cpsr);
	p->p_s.CP15_Control = CP15_DISABLE_VM(p->p_s.CP15_Control);
	p->original_priority = n;
	p->priority = n;
}

#endif

extern void test1();
extern void test2();
extern void test3();

void test(){
	termprint("Hi there!\n");
	SYSCALL(3,0,0,0);
	termprint("Back to test\n");
}

void testx(){
	for(;;)
		termprint("X");
}

void testy(){
	for(;;)
		termprint("Y");
}

void testz(){
	for(;;)
		termprint("Z");
}

void handleINT(){
	termprint("INTERRUPT!");
	HALT();
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
	terminateCurrentProc();
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
init_excarea((state_t *)INT_NEWAREA, interrupt_handler);

/*AREA TLB*/
init_excarea((state_t *)TLB_NEWAREA, handleTLB);

/*AREA PGMTRAP*/
init_excarea((state_t *)PGMTRAP_NEWAREA, handleTRAP);

/*AREA SYSKB*/
init_excarea((state_t *)SYSBK_NEWAREA, syscall_handler);

}

/*Inizializza le exception area, i PCBs e poi avvia un processo con LDST*/
int main(){
	initAreas();
	termprint("AREA DONE!\n");

	initPcbs();
	termprint("PCB DONE!\n");

	/*
	struct pcb_t* a = allocPcb();
	initProcess_KM(&a->p_s, test, 1);
	*/
	termprint("PROCESS INITIALIZED!\n");

	/*test rimozione figli*/
	struct pcb_t* x = allocPcb();
	initProcess_KM(x, test1, 1);
	//insertChild(a,x);
	struct pcb_t* y = allocPcb();
	initProcess_KM(y, test2, 2);
	//insertChild(a,y);
	struct pcb_t* z = allocPcb();
	initProcess_KM(z, test3, 3);
	//insertChild(x,z);

	initReadyQueue();

	insertReadyQueue(z);
	insertReadyQueue(y);
	insertReadyQueue(x);

	setTIMER(ACK_SLICE);
	schedule();
	/*Se arriva qua sotto dopo LDST qualcosa è andato così storto dall'aver infranto ogni regola dell'emulatore*/
	termprint("Oh no\n");

	for(;;)
		;
}
