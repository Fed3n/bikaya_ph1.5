#include "handler.h"
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
				termprint("Non sys3");
				schedule();
			break;
		}
	}
	schedule();
}

void interrupt_handler(){
	//termprint("INTERRUPT!");
	/*Se c'è un processo in corso che è stato interrotto*/
	if(currentProc != NULL){
		/*PC da decrementare di 1 word su uarm, niente su umps*/
		state_t* p = (state_t *)INT_OLDAREA;
		p->ST_PC = p->ST_PC + INT_PC*WORDSIZE;
		memcpy((state_t*) INT_OLDAREA, &(currentProc->p_s), sizeof(state_t));
	}
	int line = 0;
	while(line<=7 && !(INTERRUPT_LINE_CAUSE(getCAUSE(), line))) line++;
	/*Siccome il PLT non e’ presente su uARM, e’
	conveniente sfruttare l’interval timer su
	entrambe le piattaforme*/
	switch(line){
		case PROCESSOR_LOCAL_TIMER:
			interrupt12();
		case BUS_INTERVAL_TIMER:
			interrupt12();
		default:
			termprint("Linea non ancora implementata\n");
			HALT();
	}
}

//Aggiorno le priority, reinserisco il processo in stato ready e ripasso il controllo allo scheduler
void interrupt12(){
	/*Si aggiornano le priorità dei processi*/
	updatePriority();
	/*Si setta nuovamente il timer*/
	setTIMER(ACK_SLICE);
	/*Si richiama nuovamente lo scheduler*/
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
