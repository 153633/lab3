/* 
 * kernel.c: The main function of the kernel. 
 *
 * Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
 *          Zixuan Liu <zixuanl@andrew.cmu.edu>
 *	    Jianan Lu <jiananl@andrew.cmu.edu>
 * Date:    10/23/2013
 */


#include <exports.h>

#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/interrupt.h>
#include <arm/timer.h>
#include <arm/reg.h>

uint32_t global_data;


#include <bits/swi.h>
#include <bits/fileno.h>
#include <bits/errno.h>
//#include <exports.h>
#include <GlobalConstant.h>

char inbuf[MAXINPUTSIZE];			// The buffer of the read_sys function; used to store all the input data from the command line.
int inbufindex = 0;				// Pointer of inbuf[MAXINPUTSIZE] buffer.
int32_t read_sys(int, char*, uint32_t);		// Read syscall function, read input from STDIN
int32_t write_sys(int, char*, uint32_t);		// Write syscall function, write output to STDOUT
void    exit_sys(int);				// Exit syscall function
int32_t getSWI(int, int*);			// Dispatch the SWI to the appropriate syscall
void install_handler();				// "Wire in" my own SWI handler
void resume_handler();				// Resume the two instructions of the old SWI handler

void setup();
void install_irqhandler();
void IRQ_Handler();


int kmain(int argc, char** argv, uint32_t table)
{
	app_startup(); /* bss is valid after this point */
	global_data = table;

	setup();
	install_irqhandler();

	install_handler();			// "Wire in" my own SWI handler
	Usermode(argc, argv);			// Change the mode to user mode, set the user stack and jump to user program. 
						// Parameters argc and argv are passed from the kernel to user program successfully, although we don't use them.
	resume_handler();			// Resume the two instructions of the old SWI handler
	restore();				// Restore the u-boot context (cpsr, sp, registers)
	return ExitStatus;	
}

// Dispatch the SWI to the appropriate syscall
int32_t getSWI(int num, int *sp) {
	switch(num) {
		case READ_SWI:			
			return read_sys((int) *sp, (char *) *(sp+1), (unsigned int) *(sp+2));
		case WRITE_SWI:
			return write_sys((int) *sp, (char *) *(sp+1), (unsigned int) *(sp+2));
		case EXIT_SWI:
			exit_sys((int) *sp);
			break;
		default:
			break;
	}
	return 0;
}

int32_t read_sys(int fd, char *buf, uint32_t count) {
	// Check if it is reading from the STDIO
	if (fd != STDIN_FILENO) {                                                
                return -EBADF; 
        }
	
	// Check if the range of the buffer exits out side the range of writable memory (SDRAM)
	if (buf < (char *)0xa0000000 || (buf+count) > (char *)0xa4000000)
		return -EFAULT;

	// Check if the range of the buffer exits in some writable memory that is not writable  by a user application, namely u-boot code, heap, u-boot global data struct, abort stack and supervisor stack
	if ((buf+count) > (char *)0xa3000000)
		return -EFAULT;

	// Read from STDIN and store the input into buf. It is divided into two steps.
	// First step, store whatever inputs from STDIN to inbuf and print them to the STDOUT
	uint32_t i;
	if (inbufindex == 0) {
		for (i = 0; i < MAXINPUTSIZE; i++) {
			while(!tstc())
				continue;
			
			inbuf[i] = getc();
		
			// Check if the user enters an EOT
			if(inbuf[i] == 4)
		        {
				break;
		        }

			// Check if the user enters a backspace
			else if(inbuf[i] == 127)
			{
				// Check if the backspace is the first character
				if(i != 0)
				{
					i -= 2;
					puts("\b \b");
				}
				else
					i--;
			}
			// Check if the user enters a newline or carriage return
			else
			{
				putc(inbuf[i]);
				if((inbuf[i] == '\n') || (inbuf[i] == '\r'))
				{
					putc('\n'); 
					break;
				}	
			}				
		}// end input from STDIN
		if (i == MAXINPUTSIZE - 1)
			printf("End input from STDIN!\n");
	}

	// Second step, dump inbuf to buf. Dump "count"(the size of the buf) number of characters each time.
	for (i = 0; i < count; i++) {
		if (inbufindex < MAXINPUTSIZE) {
			buf[i] = inbuf[inbufindex];
			inbufindex++;
			// Check if the user enters an EOT
			if (buf[i] == 4) {
				inbufindex = 0;
				return i;
			}
			// Check if the user enters a newline or carriage return
			else if((buf[i] == '\n') || (buf[i] == '\r')) {
				inbufindex = 0;
				return ++i;	
			}
		}
		else {	
			inbufindex = 0;
			break;
		}
	}
	return i;
}

int32_t write_sys(int fd, char *buf, uint32_t count) {
	// Check if it is writing to the STDOUT
	if (fd != STDOUT_FILENO) {
                return -EBADF;  
	}

	// Check if the range of the buffer exits out side the range of readable memory (StrataFlash ROM or SDRAM)
	if (buf < (char *)0xa0000000 || (buf+count) > (char *)0xa4000000)
		return -EFAULT;
	
	// Check if the range of the buffer exits in some readable memory that is not readable by a user application, namely abort stack and supervisor stack
	if ((buf < (char *)0xa3ededff && (buf+count) > (char *)0xa3ededff) || (buf < (char *)0xa3000000 && (buf+count) > (char *)0xa3000000))
		return -EFAULT;

	uint32_t i;
	// Check if the last character is a carriage return; if so, print an newline instead in the last
	if(buf[count-1]=='\r')
	{
		for(i = 0; i < (count-1); i++)
			putc(buf[i]);
		printf("\n");
	}
	// Writing to the STDOUT from buf
	else
	{
		for(i = 0; i < count; i++)
			putc(buf[i]);
	}
	return count;
}

// Exit syscall
void exit_sys(int status) {
		exit(status);			// Call exit.S to exit
}

// hijack the Uboot's swi handler
void install_handler() {
	uint32_t *swi_entry = (uint32_t *)VECTORTABLE_SWI_ENTRY;			// swi vector

	if ((((*swi_entry) ^ 0xe59ff000) & 0xfffff000) == 0) {		// check the instruction at swi vector to see if it's legal 
	} else if ((((*swi_entry) ^ 0xe51ff000) & 0xfffff000) == 0) {
	} else {
		exit(0x0badc0de);					// if illegal, exit with status 0x0badc0de
	}
	
	int32_t offset = (*swi_entry) & 0x0fff;
	uint32_t jump_entry = VECTORTABLE_SWI_ENTRY + PC_CURRENTADDR_OFFSET + offset;	// address of jump table entry

	uint32_t *iaddrptr = *((uint32_t**)jump_entry);				// address of uboot's swi hander instruction

	iaddr_1 = (uint32_t *) iaddrptr;					// store the address and content of uboot's swi handler instruction 
	iaddr_2 = (uint32_t *) (iaddrptr + 1);
	instruction1 = *iaddr_1;
	instruction2 = *iaddr_2;
	
	*iaddrptr = LOAD_PC_PC_4_ENCODING;				// hijack using 'ldr pc, [pc, #-4]'
	*(iaddrptr+1) = (uint32_t)S_Handler;					// hijsck using address of our swi handler

}

// restore the uboot's swi handler instructions 
void resume_handler() {
	*iaddr_1 = instruction1;
	*iaddr_2 = instruction2;
}

// Setup for the interrupt controller and timer
void setup() {
	// Setup the interrupt controller
	reg_write(INT_ICMR_ADDR, 0x04000000);		// only enable the interrupt of os timer0
	reg_write(INT_ICLR_ADDR, 0);			// set all interrupt to IRQ

	// Setup the timer
	reg_write(OSTMR_OSCR_ADDR, 0);			// reset the OSCR
	reg_write(OSTMR_OSMR_ADDR(0), UINT32_MAX);	// set the initial OSMR0 to 0xFFFFFFFF

	reg_write(OSTMR_OSMR_ADDR(0), 20);// for test

	reg_write(OSTMR_OIER_ADDR, OSTMR_OIER_E0);	// only enable the interrupt of OSMR0
	reg_write(OSTMR_OSSR_ADDR, OSTMR_OSSR_M0);	// clear the bit initially
	puts("Setup finished!\n");
}

// hijack the Uboot's IRQ handler
void install_irqhandler() {
	uint32_t *swi_entry = (uint32_t *)VECTORTABLE_IRQ_ENTRY;			// IRQ vector

	if ((((*swi_entry) ^ 0xe59ff000) & 0xfffff000) == 0) {		// check the instruction at IRQ vector to see if it's legal 
	} else if ((((*swi_entry) ^ 0xe51ff000) & 0xfffff000) == 0) {
	} else {
		exit(0x0badc0de);					// if illegal, exit with status 0x0badc0de
	}
	
	int32_t offset = (*swi_entry) & 0x0fff;
	uint32_t jump_entry = VECTORTABLE_IRQ_ENTRY + PC_CURRENTADDR_OFFSET + offset;	// address of jump table entry

	uint32_t *iaddrptr = *((uint32_t**)jump_entry);				// address of uboot's IRQ handler instruction

	irq_iaddr_1 = (uint32_t *) iaddrptr;					// store the address and content of uboot's IRQ handler instruction 
	irq_iaddr_2 = (uint32_t *) (iaddrptr + 1);
	irq_instruction1 = *irq_iaddr_1;
	irq_instruction2 = *irq_iaddr_2;
	
	*iaddrptr = LOAD_PC_PC_4_ENCODING;				// hijack using 'ldr pc, [pc, #-4]'
	*(iaddrptr+1) = (uint32_t)IRQ_Handler;				// hijsck using address of our IRQ handler

}

void IRQ_Handler() {
	puts("Timer interrupt!\n");
}
