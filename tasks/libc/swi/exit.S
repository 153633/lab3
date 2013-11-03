@ exit.S: exit sycall wrapper
@
@ Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
@          Zixuan Liu <zixuanl@andrew.cmu.edu>
@	   Jianan Lu <jiananl@andrew.cmu.edu>
@ Date:    10/13/2013

#include <asm.h>
#include <bits/swi.h>

	.file	"exit.S"
	.text

	.global	exit
exit:
        stmfd   sp!, {r4-r11}	@ as a called routine of APCS, preserve r4 - r11 (but in fact we won't use them)
        ldr     r4, [sp, #32]	@ transfer from APCS to OABI, load the argument #5 - argument #7 to r4 - r6
        ldr     r5, [sp, #36]
        ldr     r6, [sp, #40]
        swi     EXIT_SWI	@ use syscall exit, argumens r0 is used

	
