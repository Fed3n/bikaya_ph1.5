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

#define RAMBASE    *((unsigned int *)BUS_REG_RAM_BASE)
#define RAMSIZE    *((unsigned int *)BUS_REG_RAM_SIZE)
#define RAMTOP     (RAMBASE + RAMSIZE)

#ifdef TARGET_UMPS
void termprint(char *str);

/*interrupt disabled, kernel mode, local timer on, virtual memory off*/
#define STATUS_ALL_INT_DISABLE_KM_LT(status) ((status) | STATUS_KUc | STATUS_TE)
#define STATUS_ALL_INT_ENABLE_KM_LT(status) ((status) | STATUS_IEc | STATUS_KUc | STATUS_IM_MASK | STATUS_TE)

#define TO_LOAD(status) &(status)

void init_excarea(state_t* p, void* handler){
	p->reg_sp = RAMTOP;
	p->pc_epc = (memaddr)handler;
	p->status = STATUS_ALL_INT_DISABLE_KM_LT(p->status);
}

/*inizializza processo in kernel mode, interrupt abilitati*/
void initProcess_KM(state_t* p, void* fun, int n){
	p->status = STATUS_ALL_INT_DISABLE_KM_LT(p->status);
	p->pc_epc = (memaddr)fun;
	p->reg_sp = (RAMTOP-(RAMSIZE*n));
}

#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);

#define TO_LOAD(status) &(status->a1)

void init_excarea(state_t* p, void* handler){
	p->cpsr = (p->cpsr | STATUS_SYS_MODE);
	p->cpsr = STATUS_ALL_INT_DISABLE(p->cpsr);
	p->pc = (memaddr)handler;
	p->sp = RAMTOP;
	p->CP15_Control = CP15_DISABLE_VM(p->CP15_Control);
}

/*inizializza processo in kernel mode, interrupt abilitati*/
void initProcess_KM(state_t* p, void* fun, int n){
	p->cpsr = (p->cpsr | STATUS_SYS_MODE);
	p->cpsr = STATUS_ALL_INT_ENABLE(p->cpsr);
	p->pc = (memaddr)fun;
	p->sp = (RAMTOP-(RAMSIZE*n));
	p->CP15_Control = CP15_DISABLE_VM(p->CP15_Control);
}

#endif

void test1();
void test2();
void test3();

void test(){
	termprint("Hi there!\n");
}

void testx(){
	for(;;)
		;
}

void handleINT(){
	termprint("INTERRUPT!");
	SYSCALL(0,0,0,0);
	termprint("Back to INTERRUPT!\n");
	HALT();
}

void handleTLB(){
	termprint("TLB!");
}

void handleTRAP(){
	termprint("TRAP!");
}

void handleSYSBP(){
	termprint("SYSBP!");
	HALT();
	state_t* p = (state_t*)SYSBK_OLDAREA;
	LDST(TO_LOAD(p));
}

/*Prima inizializzo tutte le aree a 0, poi assegno i campi richiesti con una macro*/
void initAreas(){
/*AREA INTERRUPT*/
ownmemset((state_t *) INT_NEWAREA, 0, sizeof(state_t));
init_excarea((state_t *)INT_NEWAREA, handleINT);

/*AREA TLB*/
ownmemset((state_t *) TLB_NEWAREA, 0, sizeof(state_t));
init_excarea((state_t *)TLB_NEWAREA, handleTLB);

/*AREA PGMTRAP*/
ownmemset((state_t *) PGMTRAP_NEWAREA, 0, sizeof(state_t));
init_excarea((state_t *)PGMTRAP_NEWAREA, handleTRAP);

/*AREA SYSKB*/
ownmemset((state_t *) SYSBK_NEWAREA, 0, sizeof(state_t));
init_excarea((state_t *)SYSBK_NEWAREA, handleSYSBP);

}

/*Quello che fa al momento è un test che rimbalza l'interrupt in una syscall che a sua volta
torna dove era rimasto l'interrupt e fa HALT()*/
int main(){
	initAreas();
	termprint("AREA DONE!\n");
	initPcbs();
	termprint("PCB DONE!\n");
	struct pcb_t* a = allocPcb();
	initProcess_KM(&a->p_s, testx, 1);
	termprint("PROCESS INITIALIZED!\n");
	state_t* p = &(a->p_s);
	LDST(TO_LOAD(p));
	termprint("Oh no\n");

	for(;;)
		;
}