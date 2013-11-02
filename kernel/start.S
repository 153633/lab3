@ start.S:	Kernel startup routine
@ Authors:	Tianyi Huang <tianyih@andrew.cmu.edu>
@          	Zixuan Liu <zixuanl@andrew.cmu.edu>
@		Jianan Lu <jiananl@andrew.cmu.edu>
@ Date:    	11/1/2013

#include <asm.h>

	.file   "start.S"

FUNC(_start)
	/*
	 * Branch immediately to main without altering the execution environment.
	 * This indirection is needed to guarantee that the program entry point is
	 * LOAD_ADDR (probably 0xa3000000) without placing strict
	 * requirements on the layout of kernel.c.  Bit first, provide main with the
	 * jump table address as one of its arguments.
	 */
	mov     r2, r8
	b       kmain

