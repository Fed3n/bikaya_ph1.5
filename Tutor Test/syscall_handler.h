#ifndef SYSCALL_HANDLER_H
#define SYSCALL_HANDLER_H
#include "pcb.h"

void syscall_handler();

void sys3();
void sys3_exec(pcb_t* root);

#endif
