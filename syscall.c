#include "syscall.h"
#include "scheduler.h"
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

#ifdef TARGET_UMPS
void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

#define SYSCALL 8
#define BREAKPOINT 9

extern pcb_t* currentProc;

void syscall_handler(){
	
	state_t* p = (state_t*)SYSBK_OLDAREA;

	termprint("handler\n");	

	if (CAUSE_REG(p) == 3) termprint(">\n");
	

	if (CAUSE_REG(p) == 4) termprint("<\n");
//if (p->CAUSE_REG < 1000000000000) termprint("<1\n");
//if (p->CAUSE_REG < 200000000000) termprint("<2\n");
if (CAUSE_REG(p)  > 3) termprint("<3\n");
if (CAUSE_REG(p)  < 4) termprint("<4\n");
if (CAUSE_REG(p)  > 1) termprint("<5\n");
if (CAUSE_REG(p)  < 5) termprint("<6\n");

	if (CAUSE_REG(p) == SYSCALL) {
		unsigned int sysNum = p->ST_A0;
		unsigned int arg1 = p->ST_A1;
		unsigned int arg2 = p->ST_A2;
		unsigned int arg3 = p->ST_A3;
		
		termprint("sysNum:3");	

		switch (sysNum){
			case 3:
				sys3();
			break;
			default:
			break;
		}
	}
	//schedule();
}


/*si occupa di terminare il processo corrente e di rimuovere tutti i figlid dalla ready queue*/
void sys3(){
	if (currentProc != NULL) 

	termprint("entry on sys3exec");	

		sys3_exec(currentProc);
	
	termprint("exit from sys3exec");		
	
	schedule();
}

/*serve una funzione ausiliaria per poterla costruire in modo ricorsivo*/
void sys3_exec(pcb_t* root){
	while (!emptyChild(root)){
		pcb_t* child = removeChild(root);
		if (child != NULL) 
			sys3_exec(child);
	}		
	outReadyQueue(root);
	terminateProc();
}
