#ifndef SCHEDULER_H
#define SCHEDULER_H

#define AGING_AMOUNT 1

pcb_t* currentProc;

void initReadyQueue();

int emptyReadyQueue();

void insertReadyQueue(pcb_t* proc);

pcb_t* removeReadyQueue();

void remove2ReadyQueue(pcb_t* proc);

pcb_t* headReadyQueue();

void terminateProc();

void updatePriority();

void schedule();
#endif
