@ read.S: read sycall wrapper
@
@ Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
@          Zixuan Liu <zixuanl@andrew.cmu.edu>
@	   Jianan Lu <jiananl@andrew.cmu.edu>
@ Date:    10/13/2013
@
@ ssize_t read(int fd, void *buf, size_t count)

#include <asm.h>
#include <bits/swi.h>
                                               
        .file   "read.S"
        .text
                        
        .global read    
read:        
        stmfd   sp!, {r4-r11}   @ as a called routine of APCS, preserve r4 - r11 (but in fact we won't use them)
        ldr     r4, [sp, #32]   @ transfer from APCS to OABI, load the argument #5 - argument #7 to r4 - r6      
        ldr     r5, [sp, #36]         
        ldr     r6, [sp, #40]         
        swi     READ_SWI        @ use syscall read, argumens r0, r1, r2 are used
        movs    r1, r0          @ check return value  
        movmi   r0, #-1         @ if r0 < 0, r0 = -1  
        ldrmi   r2, =errno      @ load the address of global variable errno                     
        rsbmi   r3, r1, #0      @ set errno = -r0     
        strmi   r3, [r2]        @ store the value back to errno                    
        ldmfd   sp!, {r4-r11}   @ reload the preserved registers
        bx      lr              @ return 
  
