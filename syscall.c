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

extern pcb_t* currentProc;

void syscall_handler(){
	state_t* p = (state_t*)SYSBK_OLDAREA;
	if (CAUSE_REG(p) == SYSCALL_EXC) {
		unsigned int sysNum = p->ST_A0;
		unsigned int arg1 = p->ST_A1;
		unsigned int arg2 = p->ST_A2;
		unsigned int arg3 = p->ST_A3;
		switch (sysNum){
			case 3:
				sys3();
			break;
			default:
				schedule();
			break;
		}
	}
	schedule();
}


/*si occupa di terminare il processo corrente e di rimuovere tutti i figlid dalla ready queue*/
void sys3(){
	if (currentProc != NULL) {
		sys3_exec(currentProc);
		currentProc = NULL;
	}
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
	freePcb(root);
}
