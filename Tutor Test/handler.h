#ifndef HANDLER_H
#define HANDLER_H
#include "pcb.h"

extern void ownmemcpy(void *src, void *dest, unsigned int size);

void syscall_handler();
void interrupt_handler();

void interrupt12();
void copystate();
void sys3();
void sys3_exec(pcb_t* root);

#endif
