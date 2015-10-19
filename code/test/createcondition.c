/* createcondition.c
 *	Simple program to test whether running a user program works.
 *	
 *	Testing the creation of a condition variable
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
    int c = CreateCondition("def", 3);
	DestroyCondition(c);
    Print("Creating condition at index: %i\n", 33, c * 1000, 0);
    Exit(0);
}
