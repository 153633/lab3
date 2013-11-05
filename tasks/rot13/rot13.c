/*
 * rot13.c: ROT13 cipher test application
 *
 * Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
 *          Zixuan Liu <zixuanl@andrew.cmu.edu>
 *  	    Jianan Lu <jiananl@andrew.cmu.edu>
 * Date:    10/13/2013
 */

#include <bits/fileno.h>
#include <unistd.h>                       
#include <stdlib.h>                       
#include <errno.h>                        
                                          
#define N 10                              
                                   
int main(int argc,char* argv[]) {        

        int readResult;			// readResult stores the return value of read
	int writeResult;		// writeResult stores the return value of write
	int writeloc;          		// writeloc stores the index of the last character we write
        char inBuffer[N];		// inBuffer is the buffer storing input
	char outBuffer[N+1]; 		// outBuffer stores the output values
	outBuffer[N] = '\n';		// outBuffer has an additional element - a new line character; when the buffer is full, it will print a new line for the new line of input
      	int tmp;


		write(1,"Firstly, let's test the argumens\n",33); //Begain to test the arguments 
		if(argc == 0)
			write(1,"There is no arguments",21);
			else
			{
				write(1,"Arguments are: ",15);
				for(tmp = 0; tmp < argc; tmp++)
				{
					while(*(argv[tmp]) != 0)
						{
							write(1, argv[tmp],1);
							argv[tmp]++;
 						}						
					write(1, "  ", 2);

				}
				write(1,"\n", 1);
			}


        while ((readResult = read(STDIN_FILENO, inBuffer, N)) > 0) {	// check if more than 0 bytes are read and no error

                int cnt;		// index of characters

		for (cnt = 0; cnt < readResult; cnt++) {
			if ((inBuffer[cnt] >= 65 && inBuffer[cnt] <= 77) || (inBuffer[cnt] >= 97 && inBuffer[cnt] <= 109)) {
				outBuffer[cnt] = inBuffer[cnt] + 13;	// A - M or a - m
			}
			else if ((inBuffer[cnt] >= 78 && inBuffer[cnt] <= 90) || (inBuffer[cnt] >= 110 && inBuffer[cnt] <= 122)) {
				outBuffer[cnt] = inBuffer[cnt] - 13;	// N - Z or n - z
			}
			else {	
				outBuffer[cnt] = inBuffer[cnt];		// others unchanged
			}
		}
		
		writeloc = 0;
		do {
			writeResult = write(STDOUT_FILENO, outBuffer + writeloc, readResult);
			if (writeResult > 0) writeloc += writeResult;	// in case write returns short count, use 'writeloc' to locate the last character we write
		} while (writeResult > 0 && writeloc != readResult);	// if it has not yet ouput all the characters that we read (write returns short count), 
									// then continue writing

		// if the buffer is full, then print a new line for new line of input
		if ((readResult == 10) && (outBuffer[N-1] != '\n') && (outBuffer[N-1] != '\r'))
			writeResult = write(STDOUT_FILENO, outBuffer + N, 1);

		if (writeResult < 0) {					// as long as error occurs in write, it terminates with status 1
			exit(1);
		}

        }       
                                  
        if (readResult == 0) {    	// if zero byte is read, exit with status 0                                                                 
                exit(0);                          
        }                                         
        else if (readResult < 0) {      // terminate with status 1
                exit(1);                                               
        }  
                  
        return 1;
}



