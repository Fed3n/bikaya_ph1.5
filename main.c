#include "const.h"
#include "pcb.h"
#include "auxfun.h"
#include "scheduler.h"
#include "handler.h"
#include "init.h"

#ifdef TARGET_UMPS
extern void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

extern void test1();
extern void test2();
extern void test3();

/*Inizializza le exception area, i PCB, mette i processi in ready queue, setta timer e poi chiama lo scheduler*/
int main(){
	initAreas();
	termprint("AREA DONE!\n");

	initPcbs();
	termprint("PCB DONE!\n");

	pcb_t* test_1 = allocPcb();
	initProcess_KM(test_1, test1, 1);

	pcb_t* test_2 = allocPcb();
	initProcess_KM(test_2, test2, 2);

	pcb_t* test_3 = allocPcb();
	initProcess_KM(test_3, test3, 3);

	termprint("PROCESSES INITIALIZED!\n");


	initReadyQueue();
	insertReadyQueue(test_1);
	insertReadyQueue(test_2);
	insertReadyQueue(test_3);
	termprint("PROCESSES QUEUED!\n");

	setTIMER(ACK_SLICE);
	termprint("Now calling scheduler...\n")
	schedule();
}
