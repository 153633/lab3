/*
 * typo.c: Echos characters back with timing data.
 *
 * Authors: Tianyi Huang <tianyih@andrew.cmu.edu>
 *          Zixuan Liu <zixuanl@andrew.cmu.edu>
 *  	    Jianan Lu <jiananl@andrew.cmu.edu>
 * Date:    11/5/2013
 */

#include <bits/fileno.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define N 10

int main(int argc, char** argv)
{
	char Buffer[N];
	int readResult = 0;
	int writeResult = 0;		// writeResult stores the return value of write
	int writeloc = 0;
	int Time = 0;
	int timeSpentInt = 0;
	int timeSpentFloat = 0;
/*
	printf("start!");
	printf("\n");
	
	Time = 50;
	timeSpentInt = (int) (Time/100);
	timeSpentFloat = ((int) (Time/10)) - timeSpentInt*10;
	printf("Time spent: %d",timeSpentInt);
	printf(".");
	printf("%d",timeSpentFloat);
	printf("\n");

	Time = 150;
	timeSpentInt = (int) (Time/100);
	timeSpentFloat = ((int) (Time/10)) - timeSpentInt*10;
	printf("Time spent: %d",timeSpentInt);
	printf(".");
	printf("%d",timeSpentFloat);
	printf("\n");
	
	Time = 250;
	timeSpentInt = (int) (Time/100);
	timeSpentFloat = ((int) (Time/10)) - timeSpentInt*10;
	printf("Time spent: %d",timeSpentInt);
	printf(".");
	printf("%d",timeSpentFloat);
	printf("\n");
*/
	Time = time();
	printf("%d",Time);
	printf("\n");
	while ((readResult = read(STDIN_FILENO, Buffer, N)) > 0) {
		Time = time() - Time;
		printf("%d",Time);
		printf("\n");
		timeSpentInt = (int) (Time/100);
		timeSpentFloat = ((int) (Time/10)) - timeSpentInt*10;
		writeloc = 0;
		do {
			writeResult = write(STDOUT_FILENO, Buffer + writeloc, readResult);
			if (writeResult > 0) writeloc += writeResult;	// in case write returns short count, use 'writeloc' to locate the last character we write
		} while (writeResult > 0 && writeloc != readResult);	// if it has not yet ouput all the characters that we read (write returns short count), 
									// then continue writing
		//write(STDOUT_FILENO, Buffer, readResult);

		printf("Time spent: ");
		printf("%d",timeSpentInt);
		printf(".");
		printf("%d",timeSpentFloat);
		printf("\n");
	
	}
	
	return 0;
}
