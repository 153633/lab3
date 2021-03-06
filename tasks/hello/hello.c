/** @file hello.c
 *
 * @brief Prints out Hello world using the syscall interface.
 *
 * Links to libc.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date   2008-10-29
 */
#include <unistd.h>

int main(int argc, char** argv)
{
	const char hello[] = "Hello World\r\n";
	write(STDOUT_FILENO, hello, sizeof(hello) - 1);
	int i = 0;
	int j = 0;
	for(i=0;i<100;i++)
	{
		write(STDOUT_FILENO, hello, sizeof(hello) - 1);
		for(j=0;j<100000;j++)
			j++;
	}
	sleep(10000);
	for(i=0;i<100;i++)
	{
		write(STDOUT_FILENO, hello, sizeof(hello) - 1);
		for(j=0;j<100000;j++)
			j++;
	}
	return 0;
}
