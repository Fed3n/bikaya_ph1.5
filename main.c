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
#include "syscall.h"

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

void test1();
void test2();
void test3();

void test(){
	termprint("Hi there!\n");
	SYSCALL(3,0,0,0);
	termprint("Back to test\n");
}

void testx(){
	termprint("I'm testX!\n");
	for(;;)
		;
}

void testy(){
	termprint("I'm testY!\n");
	for(;;)
		;
}

void testz(){
	termprint("I'm testZ!\n");
	for(;;)
		;
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
init_excarea((state_t *)INT_NEWAREA, handleINT);

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

	struct pcb_t* a = allocPcb();
	initProcess_KM(&a->p_s, test, 1);
	termprint("PROCESS INITIALIZED!\n");

	/*test rimozione figli*/
	struct pcb_t* x = allocPcb();
	initProcess_KM(&x->p_s, testx, 2);
	//insertChild(a,x);
	struct pcb_t* y = allocPcb();
	initProcess_KM(&y->p_s, testy, 3);
	//insertChild(a,y);
	struct pcb_t* z = allocPcb();
	initProcess_KM(&z->p_s, testz, 4);
	//insertChild(x,z);

	/*Qua nella versione finale immagino andrà chiamato schedule() (e forse prima inizializzato il timer)*/
	/*
	state_t* p = &(a->p_s);
	LDST(TO_LOAD(p));
	*/
	initReadyQueue();
//	insertReadyQueue(a);

	insertReadyQueue(z);
	insertReadyQueue(y);
	insertReadyQueue(x);
	insertReadyQueue(a);
	
	schedule();
	/*Se arriva qua sotto dopo LDST qualcosa è andato così storto dall'aver infranto ogni regola dell'emulatore*/
	termprint("Oh no\n");

	for(;;)
		;
}
