// S_Handler.S: gets swi number and calls dispatcher to deal with syscalls
//
// Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
//          Zixuan Liu <zixuanl@andrew.cmu.edu>
//	    Jianan Lu <jiananl@andrew.cmu.edu>
// Date:    10/23/2013
    
	.file	"IRQ_Handler.S"
	.text

	.global	IRQ_Handler
                                                               
IRQ_Handler:
	ldr	sp, =0xa1000000                                                                     
        sub     sp, sp, #4		@ prepare stack to store spsr at the top
        stmfd   sp!, {r0-r12, lr}       @ store registers
        mrs     r2, spsr                                       
        str     r2, [sp, #14*4]         @ save spsr on stack                                                                     
        bl      C_IRQ_Handler		@ call dispatcher                          
        ldr     r2, [sp, #14*4]                                                 
        msr     spsr, r2                                                        
        ldmfd   sp!, {r0-r12, lr}	@ restore spsr and other registers
        add     sp, sp, #4              @ restore sp                 
        movs    pc, lr                  @ finish syscalls
