#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"

void syscall_handler();

void sys3();
void sys3_exec(pcb_t* root);

#endif
