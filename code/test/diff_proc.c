/* diff_proc.c
 *	Helper file to test lock and cond validation.
 *
 *	This will make sure you can't access a lock or condition that is in a different
 *	address space.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int main() {
	DestroyCondition(0); /* can't destroy a condition in different addrspace */
	DestroyLock(0); /* can't destroy lock in different addrspace */
	Exit(0);
}