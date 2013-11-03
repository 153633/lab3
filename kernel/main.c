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

#include <bits/swi.h>
#include <bits/fileno.h>
#include <bits/errno.h>

#include <GlobalConstant.h>

uint32_t global_data;

char inbuf[MAXINPUTSIZE];			// The buffer of the read_sys function; used to store all the input data from the command line.
int inbufindex = 0;				// Pointer of inbuf[MAXINPUTSIZE] buffer.

int32_t read_sys(int, char*, uint32_t);		// Read syscall function, read input from STDIN
int32_t write_sys(int, char*, uint32_t);		// Write syscall function, write output to STDOUT
void    exit_sys(int);				// Exit syscall function
uint32_t time_sys();
void	 sleep_sys(uint32_t);

int32_t getSWI(int, int*);			// Dispatch the SWI to the appropriate syscall
void install_handler();				// "Wire in" my own SWI handler
void resume_handler();				// Resume the two instructions of the old SWI handler

void setup();
void install_irqhandler();
void IRQ_Handler();
void C_IRQ_Handler();
void test();


int kmain(int argc, char** argv, uint32_t table)
{
	app_startup(); /* bss is valid after this point */
	global_data = table;
	printf("Start!\n");
	//test();
	//printf("Enable IRQ finished!\n");
	


/*
	printf("Start setup!\n");
	setup();
<<<<<<< HEAD
=======
	install_irqhandler();
>>>>>>> f1128302919db4482afeaacf0cf4cec5e3647817

	printf("ICMR: 0x%x\n", reg_read(INT_ICMR_ADDR));
	printf("ICLR: 0x%x\n", reg_read(INT_ICLR_ADDR));
	printf("ICPR: 0x%x\n", reg_read(INT_ICPR_ADDR));
	printf("OSCR: %d\n", reg_read(OSTMR_OSCR_ADDR));
	printf("OSMR0: %d\n", reg_read(OSTMR_OSMR_ADDR(0)));
	printf("OSSR: 0x%x\n", reg_read(OSTMR_OSSR_ADDR));
	printf("OIER: 0x%x\n", reg_read(OSTMR_OIER_ADDR));

	printf("OSCR: %d\n", reg_read(OSTMR_OSCR_ADDR));
	printf("OSSR: %d\n", reg_read(OSTMR_OSSR_ADDR));
	printf("ICPR: %d\n", reg_read(INT_ICPR_ADDR));

	printf("OSCR: %d\n", reg_read(OSTMR_OSCR_ADDR));
	printf("OSSR: %d\n", reg_read(OSTMR_OSSR_ADDR));
	printf("ICPR: %d\n", reg_read(INT_ICPR_ADDR));

	printf("OSCR: %d\n", reg_read(OSTMR_OSCR_ADDR));
	printf("OSSR: %d\n", reg_read(OSTMR_OSSR_ADDR));
	printf("ICPR: %d\n", reg_read(INT_ICPR_ADDR));

*/

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
		case TIME_SWI:
			return time_sys();
		case SLEEP_SWI:
			sleep_sys((uint32_t) *sp);
			return 0; 
		default:
			break;
	}
	return 0;
}


uint32_t time_sys() {
	return system_time;
}

void sleep_sys(uint32_t msec) {

	printf("sleeping %d ms\n", msec);
	printf("start from system time: %d\n", system_time);

	reg_write(OSTMR_OIER_ADDR, 0);			// mask, make sure there's no timer interrupts
	reg_write(OSTMR_OSSR_ADDR, OSTMR_OSSR_M0);	// clear the bit, otherwise there might be an interrupt after syscall

	reg_write(OSTMR_OSCR_ADDR, 0xff0fffff);		// for test
	printf("Assume oscr starts from %x\n",reg_read(OSTMR_OSCR_ADDR));

	uint32_t start_time = reg_read(OSTMR_OSCR_ADDR)/32500;		// start time
	printf("start time: %d\n", start_time);
	uint32_t current_time = start_time;				// current time
	uint32_t tmp_time;						// tmp_time stores oscr
	uint32_t current_time_base = 0;					// 'base' is for continuing computing current time after oscr reaches UINT32_MAX

	while (10 * (current_time - start_time) < msec) {
		tmp_time = reg_read(OSTMR_OSCR_ADDR);
		if (tmp_time > UINT32_MAX - 3250) {			// need to be changed, 3250 could be any reasonable number
			current_time_base = current_time_base + tmp_time/32500;		// update 'base'
			printf("oscr booooooommmm at %x\n", tmp_time);
			printf("base: %d\n", current_time_base);
			reg_write(OSTMR_OSCR_ADDR, 0);			// reset the OSCR after reaching UINT32_MAX
			//printf("oscr: %d\n",reg_read(OSTMR_OSCR_ADDR));
			tmp_time = 0;
		}
		current_time = current_time_base + tmp_time/32500;	// update current time
	}

	printf("end time: %d\n", current_time);
	system_time = system_time + current_time - start_time;		// update system time

	reg_write(OSTMR_OIER_ADDR, OSTMR_OIER_E0);			// unmask OIER

	//printf("oscr: %d\n",reg_read(OSTMR_OSCR_ADDR));
	printf("end sleeping...current system time: %d\n", system_time);
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
	int *swi_entry = (int *)VECTORTABLE_SWI_ENTRY;			// swi vector

	if ((((*swi_entry) ^ 0xe59ff000) & 0xfffff000) == 0) {		// check the instruction at swi vector to see if it's legal 
	} else if ((((*swi_entry) ^ 0xe51ff000) & 0xfffff000) == 0) {
	} else {
		exit(0x0badc0de);					// if illegal, exit with status 0x0badc0de
	}
	
	int offset = (*swi_entry) & 0x0fff;
	int jump_entry = VECTORTABLE_SWI_ENTRY + PC_CURRENTADDR_OFFSET + offset;	// address of jump table entry

	int *iaddrptr = *((int**)jump_entry);				// address of uboot's swi hander instruction

	iaddr_1 = (int *) iaddrptr;					// store the address and content of uboot's swi handler instruction 
	iaddr_2 = (int *) (iaddrptr + 1);
	instruction1 = *iaddr_1;
	instruction2 = *iaddr_2;
	
	*iaddrptr = LOAD_PC_PC_4_ENCODING;				// hijack using 'ldr pc, [pc, #-4]'
	*(iaddrptr+1) = (int)S_Handler;					// hijsck using address of our swi handler

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
	printf("OSCR: %d\n", reg_read(OSTMR_OSCR_ADDR));
	reg_write(OSTMR_OSMR_ADDR(0), 32500);	// set the initial OSMR0 to 32500, so it will trigger an interrupt every 10ms

	//reg_write(OSTMR_OSMR_ADDR(0), 20);// for test

	reg_write(OSTMR_OIER_ADDR, OSTMR_OIER_E0);	// only enable the interrupt of OSMR0
	reg_write(OSTMR_OSSR_ADDR, OSTMR_OSSR_M0);	// clear the bit initially
	puts("Setup finished!\n");
}

// hijack the Uboot's IRQ handler
void install_irqhandler() {
	int *swi_entry = (int *)VECTORTABLE_IRQ_ENTRY;			// IRQ vector

	if ((((*swi_entry) ^ 0xe59ff000) & 0xfffff000) == 0) {		// check the instruction at IRQ vector to see if it's legal 
	} else if ((((*swi_entry) ^ 0xe51ff000) & 0xfffff000) == 0) {
	} else {
		exit(0x0badc0de);					// if illegal, exit with status 0x0badc0de
	}
	
	int offset = (*swi_entry) & 0x0fff;
	int jump_entry = VECTORTABLE_IRQ_ENTRY + PC_CURRENTADDR_OFFSET + offset;	// address of jump table entry

	int *iaddrptr = *((int**)jump_entry);				// address of uboot's IRQ handler instruction

	irq_iaddr_1 = (int *) iaddrptr;					// store the address and content of uboot's IRQ handler instruction 
	irq_iaddr_2 = (int *) (iaddrptr + 1);
	irq_instruction1 = *irq_iaddr_1;
	irq_instruction2 = *irq_iaddr_2;
	
	*iaddrptr = LOAD_PC_PC_4_ENCODING;				// hijack using 'ldr pc, [pc, #-4]'
	*(iaddrptr+1) = (int)IRQ_Handler;				// hijsck using address of our IRQ handler

}

void C_IRQ_Handler() {
	system_time++;
	printf("%d\n", system_time);
	printf("Timer interrupt!\n");
	reg_write(OSTMR_OSCR_ADDR, 0);			// reset the OSCR
	reg_set(OSTMR_OSSR_ADDR, 1);
	printf("Return from interrupt!\n");
}
