@ Usermode.S: Change to usermode and execute the user program
@
@ Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
@          Zixuan Liu <zixuanl@andrew.cmu.edu>
@	       Jianan Lu <jiananl@andrew.cmu.edu>
@ Date:    10/23/2013


        .file	"test.S"
	.text
	.global  test

test:         
	mrs     r0, cpsr  
	mov	r1, #0x80
	bic	r0, r0, r1
        msr   	cpsr, r0
	bx	lr  
        
		

    
