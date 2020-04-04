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
#define RAM_FRAMESIZE 4096
#define WORDSIZE 4

#ifdef TARGET_UMPS
void termprint(char *str);

/*interrupt disabled, kernel mode, local timer on, virtual memory off*/
#define STATUS_ALL_INT_DISABLE_KM_LT(status) ((status) | (STATUS_TE))
#define STATUS_ALL_INT_ENABLE_KM_LT(status) ((status) | (STATUS_IEc) | STATUS_IM(1) | (STATUS_TE))

/*macro da passare a LDST*/
#define TO_LOAD(status) (status)
/*macro per accedere al pc da state_t*/
#define ST_PC pc_epc

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 1

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

/*macro per usare LDST su uarm*/
#define TO_LOAD(status) &(status->a1)
/*macro per accedere al pc da state_t*/
#define ST_PC pc

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 0

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

static struct list_head queue;
static pcb_t *current;
static struct list_head *appoggio;

void test1();
void test2();
void test3();

void testx(){
	for(;;)
		;
}

void state(state_t* source, state_t* dest){
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
    for(int i = 0; i<30: i++) dest->gpr[i] = source->gpr[i];
    dest->hi = source->hi;
    dest->lo = source->lo;
    #endif
}
	

void test(){
	termprint("Hi there!\n");
	SYSCALL(0,0,0,0);
	termprint("Back to test\n");
	HALT();
}

void handleINT(){
	termprint("\nINTERRUPT!\n");
	state_t* p = (state_t *)INT_OLDAREA;
	/*prima di ridare controllo al processo qua dovremmo diminuire di 1 word il pc a uarm, niente su umps*/
	state(p, &current->p_s);
	insertProcQ(&queue,current);
	setTIMER(3000);
	schedule();
	HALT();
}

void handleTLB(){
	termprint("TLB!\n");
	HALT();
}

void handleTRAP(){
	termprint("TRAP!\n");
	HALT();
}

void handleSYSBP(){
	termprint("SYSBP!\n");
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

void schedule(){
	if(!emptyProcQ(&queue)) {
		current = removeProcQ(&queue);
		current->priority = current->original_priority;
		//Meccanismo di aging per prevenire starvation
		list_for_each(appoggio, &queue) {
			container_of(appoggio, pcb_t, p_next)->priority++;
		}
		state_t* p = &(current->p_s);
		termprint("ALL OK!\n");
		setTIMER(3000);
		LDST(TO_LOAD(p));
		termprint("Oh no\n");
	} else HALT();
}

/*Inizializza le exception area, i PCBs e poi avvia un processo con LDST*/
int main(){
	initAreas();
	termprint("AREA DONE!\n");

	initPcbs();
	termprint("PCB DONE!\n");

	mkEmptyProcQ(&queue);

	static pcb_t *a[3];

	//Per questa fase bastano 3 processi di test
	for(int i = 1; i <= 3; i++) {
		a[i] = allocPcb();
		initProcess_KM(&(a[i]->p_s), test, i);
		insertProcQ(&queue,a[i]);
		termprint("PROCESS INITIALIZED\n");
	}
	termprint("ALL INITIALIZED!\n");
	
	schedule();

	for(;;)
		;
}
