#ifndef SCHEDULER_H
#define SCHEDULER_H

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

#include "const.h"
#include "pcb.h"
#include "types_bikaya.h"
#include "auxfun.h"
#include "scheduler.h"

void state(state_t* source, state_t* dest);

void interrupt();

#endif
