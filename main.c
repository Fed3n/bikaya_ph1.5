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
	HALT();
}

void handleINT(){
	termprint("INTERRUPT!");
	/*prima di ridare controllo al processo qua dovremmo diminuire di 1 word il pc a uarm, niente su umps*/
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

	state_t* p = &(a->p_s);
	LDST(TO_LOAD(p));
	/*Se arriva qua sotto dopo LDST qualcosa è andato così storto dall'aver infranto ogni regola dell'emulatore*/
	termprint("Oh no\n");

	for(;;)
		;
}