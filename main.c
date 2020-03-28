#ifdef TARGET_UMPS
#include "libumps.h"
#include "arch.h"
#include "exc_area.h"
#include "cp0.h"
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

#define STATE_T_SP(location) *(unsigned int *)(location + 30*sizeof(unsigned int))
#define STATE_T_PC(location) *(unsigned int *)(location + 3*sizeof(unsigned int))
#define STATE_T_STATUS(location) *(unsigned int *)(location + 2*sizeof(unsigned int))
/*interrupt disabled, kernel mode, local timer on, virtual memory off*/
#define STATUS_ALL_INT_DISABLE_KM_LT(status) ((status) | STATUS_KUc | STATUS_TE) 
#define STATUS_ALL_INT_ENABLE_KM_LT(status) ((status) | STATUS_IEc | STATUS_KUc | STATUS_TE) 

/*SP is handler address, interrupt disabled, kernel mode, local timer on, virtual memory off*/
#define INIT_EXCAREA(location, handler) { 													\
	STATE_T_SP(location) = RAMTOP;															\
	STATE_T_PC(location) = (memaddr)handler;												\
	STATE_T_STATUS(location) = STATUS_ALL_INT_DISABLE_KM_LT(STATE_T_STATUS(location));		\
}

void initProcess_KM(state_t* s_t, memaddr fun, int n){
	s_t->status = STATUS_ALL_INT_ENABLE_KM_LT(s_t->status);
	s_t->pc_epc = fun;
	s_t->reg_sp = (RAMTOP-(RAMSIZE*n));
}

#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);

#define STATE_T_SP(location) *(unsigned int *)(location + 13*sizeof(unsigned int))
#define STATE_T_PC(location) *(unsigned int *)(location + 15*sizeof(unsigned int))
#define STATE_T_STATUS(location) *(unsigned int *)(location + 16*sizeof(unsigned int))
#define STATE_T_CONTROL(location) *(unsigned int *)(location + 17*sizeof(unsigned int))

#define INIT_EXCAREA(location, handler) { 										\
	STATE_T_SP(location) = RAMTOP;												\
	STATE_T_PC(location) = (memaddr)handler;									\
	STATE_T_STATUS(location) = (STATE_T_STATUS(location) | STATUS_SRV_MODE);	\
	STATE_T_STATUS(location) = STATUS_ALL_INT_DISABLE(STATE_T_STATUS(location));\
	STATE_T_CONTROL(location) = CP15_DISABLE_VM(STATE_T_CONTROL(location));		\
}

void initProcess_KM(state_t* s_t, memaddr fun, int n){
	s_t->cpsr = (s_t->cpsr | STATUS_SRV_MODE);
	s_t->cpsr = STATUS_ALL_INT_ENABLE(s_t->cpsr);
	s_t->pc = fun;
	s_t->sp = (RAMTOP-(RAMSIZE*n));
	s_t->CP15_Control = CP15_DISABLE_VM(s_t->CP15_Control);
}

#endif

void test1();
void test2();
void test3();

void testx(){
	for(;;)
		termprint("hello");
}

void handleINT(){
	termprint("INTERRUPT!");
}

void handleTLB(){
	termprint("TLB!");
}

void handleTRAP(){
	termprint("TRAP!");
}

void handleSYSBP(){
	termprint("SYSBP!");
}

/*Prima inizializzo tutte le aree a 0, poi assegno i campi richiesti con una macro*/
void initAreas(){
/*AREA INTERRUPT*/
ownmemset((unsigned int *) INT_NEWAREA, 0, sizeof(state_t));
INIT_EXCAREA(INT_NEWAREA, handleINT)

/*AREA TLB*/
ownmemset((unsigned int *) TLB_NEWAREA, 0, sizeof(state_t));
INIT_EXCAREA(TLB_NEWAREA, handleTLB)

/*AREA PGMTRAP*/
ownmemset((unsigned int *) PGMTRAP_NEWAREA, 0, sizeof(state_t));
INIT_EXCAREA(PGMTRAP_NEWAREA, handleTRAP)

/*AREA SYSKB*/
ownmemset((unsigned int *) SYSBK_NEWAREA, 0, sizeof(state_t));
INIT_EXCAREA(SYSBK_NEWAREA, handleSYSBP)

}

int main(){
	termprint("HELLO WORLD!! :)\n");

	initAreas();
	termprint("AREA DONE!\n");
	initPcbs();
	termprint("PCB DONE!\n");
	struct pcb_t* a = allocPcb();
	initProcess_KM(&a->p_s, (memaddr)testx, 1);
	termprint("PROCESS INITIALIZED!\n");
	LDST(&(a->p_s));
	termprint("Oh no\n");

	for(;;)
		;
}