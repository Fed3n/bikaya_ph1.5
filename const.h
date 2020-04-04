#ifndef _CONST_H
#define _CONST_H

/**************************************************************************** 
 *
 * This header file contains the global constant & macro definitions.
 * 
 ****************************************************************************/

/* Maxi number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20

#define UPROCMAX 3  /* number of usermode processes (not including master proc
		       and system daemons */

#define	HIDDEN static
#define	TRUE 	1
#define	FALSE	0
#define ON 		1
#define OFF 	0
#define EOS '\0'

#define DEV_PER_INT 8 /* Maximum number of devices per interrupt line */

#define CR 0x0a   /* carriage return as returned by the terminal */

#define RAMBASE    *((unsigned int *)BUS_REG_RAM_BASE)
#define RAMSIZE    *((unsigned int *)BUS_REG_RAM_SIZE)
#define RAMTOP     (RAMBASE + RAMSIZE)
#define RAM_FRAMESIZE 4096
#define WORDSIZE 4

/**************************************************************
*
* UARM and UMPS specific global constants & macro definitions
*
**************************************************************/
#ifdef TARGET_UMPS
/*interrupt disabled, kernel mode, local timer on, virtual memory off*/
#define STATUS_ALL_INT_DISABLE_KM_LT(status) ((status) | (STATUS_TE))
#define STATUS_ALL_INT_ENABLE_KM_LT(status) ((status) | (STATUS_IEc) | STATUS_IM(1) | (STATUS_TE))

/*macro da passare a LDST*/
#define TO_LOAD(status) (status)
/*macro per accedere al pc da state_t*/
#define ST_PC pc_epc

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 1
#endif

#ifdef TARGET_UARM
/*macro per usare LDST su uarm*/
#define TO_LOAD(status) &(status->a1)
/*macro per accedere al pc da state_t*/
#define ST_PC pc

/*word count da modificare prima di chiamare LDST dopo un'eccezione*/
#define SYSBP_PC 0
#endif

#endif
