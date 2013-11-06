@ restore.S:Restore cpsr and restore the registers used in Usermode
@          	Restore the supervisor mode sp
@ Authors: 	Tianyi Huang <tianyih@andrew.cmu.edu>
@          	Zixuan Liu <zixuanl@andrew.cmu.edu>
@	       	Jianan Lu <jiananl@andrew.cmu.edu>
@ Date:    	10/23/2013

        .file	"restore.S"
		.text
	    .global restore

restore:
		ldr     r5, =oldspsr
		ldr     r4, [r5]         @ restore supervisor modecpsr        

		msr     cpsr, r4
		ldr     r5, =oldsp
		ldr	sp, [r5]             @ restore supervisor sp
		 
		ldmfd   sp!, {r4-r11}    @ restore register include R8 that may be used in usermode}

		ldr	r3, =rr8		@ added by lu
		ldr	r8, [r3]

		bx      lr               @ return to kernel main
