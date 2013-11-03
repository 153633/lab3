@ crt0.S: Execution startup routine
@
@ Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
@          Zixuan Liu <zixuanl@andrew.cmu.edu>
@	   Jianan Lu <jiananl@andrew.cmu.edu>
@ Date:    10/13/2013

#include <asm.h>

	.file	"crt0.S"
	.text

	.global	_start

_start:
	ldr	r0, [sp]	@ load argc
	add	r1, sp, #4	@ load argv
	bl 	main		@ call main routine, return value at r0
	bl	exit		@ call exit, argument is r0

