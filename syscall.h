#ifndef SYSCALL_H
#define SYSCALL_H
#include "pcb.h"

void sys3();
static void sys3_exec(pcb_t* root);

#endif
