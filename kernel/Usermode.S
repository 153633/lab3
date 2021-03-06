@ Usermode.S: Change to usermode and execute the user program
@
@ Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
@          Zixuan Liu <zixuanl@andrew.cmu.edu>
@	       Jianan Lu <jiananl@andrew.cmu.edu>
@ Date:    10/23/2013


        .file	"Usermode.S"
	    .text
		.global  Usermode

Usermode:         
		stmfd   sp!, {r4-r11}   @ Called routine of apcs include the r8

		ldr	r4, =rr8	@ added by lu
		str	r8, [r4]
	
		ldr		r4, =link       @ Store the lr to a globle value
		str     lr, [r4]        @ Restore lr in the function exit()

		ldr		r5, =oldsp      @ Store the supervisor mode sp to a globle value
		str     sp, [r5]        @ Restore the sp in the function restore()xs
		
		mrs     r5, cpsr  
		ldr		r4, =oldspsr    @ Store the supervisor cpsr to a globel value
		str     r5, [r4]        @ Restore it in the function restore       
		bic     r5, r5, #0xff   
	    	orr     r5, r5, #0x50
        	msr   	cpsr, r5        @ set cpsr to Usermode and disable the FIQ and IRQ  


        	ldr 	sp, =0xa3000000	@ The user stack starts at 0xa3000000
		mov 	r3, #0
        	str 	r3, [sp]		@ Set the User stack bottom to NULL

		mov 	r2, r0
label1:
  		cmp 	r2, #0
		beq		label2
		sub 	sp,	sp,	#4
		sub 	r2,	r2,	#1
		b		label1			@ Set up space for user stack to get the arguments
		   

label2:
		sub 	sp, sp, #4
		str		r0,	[sp]
		mov		r6,	sp			@ Protect the Usermode stack sp starting address 
		add		sp,	sp,	#4		

label3:
		cmp 	r0, #0
		beq 	label4
		ldr 	r5, [r1]	 
		str 	r5, [sp]
		add 	sp, sp, #4
		add 	r1, r1, #4
		sub 	r0, r0, #1
		b		label3			@ Push the arguments to the Usrstack
								
		 	
label4: 

		mov	sp, r6  		@ set the user stack the stack has prepared for passing arguments
		                        @ sp contains the argc and sp+4 is the argv[0]
								@ We have tested that the arguments from kernel main
								@ have successfully been passed to the User main

		bl	setup 		  
		ldr     pc, =0xa0000000	@ go to userprogram
        
		

    
