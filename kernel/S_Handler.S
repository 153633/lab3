// S_Handler.S: gets swi number and calls dispatcher to deal with syscalls
//
// Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
//          Zixuan Liu <zixuanl@andrew.cmu.edu>
//	    Jianan Lu <jiananl@andrew.cmu.edu>
// Date:    10/23/2013
    
	.file	"S_Handler.S"
	.text

	.global	S_Handler
                                                               
S_Handler:                                                                      
	sub	sp, sp, #4		@ prepare stack to store spsr at the top
	stmfd	sp!, {r0-r12, lr}	@ store registers
	mrs	r2, spsr                                       
	str	r2, [sp, #14*4]		@ save spsr on stack                                        
	mov	r1, sp                                                          
	ldr	r0, [lr, #-4]		@ get swi number from the user app                    
	bic	r0, r0, #0xff000000           
	bl	getSWI			@ call dispatcher
	ldr	r5, =value		
	str	r0, [r5]		@ store return value of syscalls                          
	ldr	r2, [sp, #14*4]                                                 
	msr	spsr, r2                                                        
	ldmfd	sp!, {r0-r12, lr}	@ restore spsr and other registers
        ldr	r0, =value
	ldr	r0, [r0]		@ prepare return value
	add	sp, sp, #4		@ restore sp                 
	movs	pc, lr			@ finish syscalls
