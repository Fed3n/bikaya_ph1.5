#include "interrupt_handler.h"
#include "syscall_handler.h"
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

extern void ownmemcpy(void *src, void *dest, unsigned int size);
extern pcb_t* currentProc;

void syscall_handler(){
	//recupero dell'old area
	state_t* p = (state_t*)SYSBK_OLDAREA;
	//controllo se l'eccezione sollevata è una system call
	if (CAUSE_REG(p) == SYSCALL_EXC) {
		//recupero del tipo e dei parametri della systemcall
		unsigned int sysNum = p->ST_A0;
		unsigned int arg1 = p->ST_A1;
		unsigned int arg2 = p->ST_A2;
		unsigned int arg3 = p->ST_A3;

		switch (sysNum){
			case 3:
				sys3();
			break;
			default:
				termprint("Non sys3");
				schedule();
			break;
		}
	}
	schedule();
}

//si occupa di terminare il processo corrente e di rimuovere tutti i figli dalla ready queue
void sys3(){
	if (currentProc != NULL) {
		sys3_exec(currentProc);
		currentProc = NULL;
	}
	schedule();
}

//funzione ausiliaria utilizzata per poter realizzare la ricorsione
void sys3_exec(pcb_t* root){
	while (!emptyChild(root)){
		pcb_t* child = removeChild(root);
		if (child != NULL) 
			sys3_exec(child);
	}		
	outReadyQueue(root);
	freePcb(root);
}
