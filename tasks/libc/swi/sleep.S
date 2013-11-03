/** @file sleep.S
 *
 * @brief sleep sycall wrapper
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-10-31
 */

#include <asm.h>
#include <bits/swi.h>

	.file "sleep.S"

FUNC(sleep)
	stmfd   sp!, {r4-r11}   @ as a called routine of APCS, preserve r4 - r11 (but in fact we won't use them)
        ldr     r4, [sp, #32]   @ transfer from APCS to OABI, load the argument #5 - argument #7 to r4 - r6      
        ldr     r5, [sp, #36]         
        ldr     r6, [sp, #40]         
        swi     SLEEP_SWI       @ use syscall read, argument r0 is used                    
        ldmfd   sp!, {r4-r11}   @ reload the preserved registers
        bx      lr              @ return 
