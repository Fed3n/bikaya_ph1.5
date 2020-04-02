#ifndef SCHEDULER_H
#define SCHEDULER_H

#define CONTINUE 0
#define UPDATE_PRIORITY 1
#define INIT 2

void initReadyQueue();

void updatePriority();

void schedule();