#ifdef TARGET_UMPS
#include "libumps.h"
#include "arch.h"
#endif

#ifdef TARGET_UARM
#include "libuarm.h"
#include "arch.h"
#endif

#include "const.h"
#include "pcb.h"

#ifdef TARGET_UMPS
void termprint(char *str);
#endif

#ifdef TARGET_UARM
#define termprint(str) tprint(str);
#endif

void test1();
void test2();
void test3();

int main(){
	termprint("HELLO WORLD!! :)");
	for(;;)
		;
}