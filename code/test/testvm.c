/* testvm.c
 *	Simple program to test virtual memory.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int main()
{
	Write("Running matmult 1\n", 18, ConsoleOutput);
	Exec("../test/matmult", 15);
	Write("Running matmult 2\n", 18, ConsoleOutput);
	Exec("../test/matmult", 15);
    Exit(0);
}