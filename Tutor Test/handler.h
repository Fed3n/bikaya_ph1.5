#ifndef HANDLER_H
#define HANDLER_H
#include "pcb.h"

void syscall_handler();
void interrupt_handler();

void interrupt12();
void sys3();
void sys3_exec(pcb_t* root);

#endif
