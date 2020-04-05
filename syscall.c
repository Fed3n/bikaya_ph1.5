#include "syscall.h"
#include "scheduler.h"

/*si occupa di terminare il processo corrente e di rimuovere tutti i figlid dalla ready queue*/
void sys3(){
	if (currentProc != NULL) 
		sys3_exec(currentProc);
	schedule();
}

/*serve una funzione ausiliaria per poterla costruire in modo ricorsivo*/
static void sys3_exec(pcb_t* root){
	while (!emptyChild(root)){
		pcb_t* child = removeChild(root);
		if (child != NULL) 
			sys3_exec(child);
	}		
	remove2ReadyQueue(root);
	terminateProc2();
}
