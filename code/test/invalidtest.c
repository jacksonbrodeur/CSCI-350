/* invalidtest.c
 *	Simple program to test invalid values.
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

	/* TEST CASES
		Need to test: (for all calls except Create)
			- below and above range
			- null object at index
			- different addr space
	
		Only in create lock/condition do we check anything else - MAXFILENAME (=256) & bad vaddr
	*/

	// Locks - Create
	int l1 = CreateLock("l1", -1); // below range
	int l2 = CreateLock("l2", 300); // above range
	/* code to test bad vaddr */ // can we test bad vaddr?

	// Locks - the rest
	DestroyLock(-1); // below range
	DestroyLock(100001); // above range
	DestroyLock(9999); // lock should be NULL
	// diff addr space (exec)


	// Conditions - Create
    int c1 = CreateCondition("c1", -1); // below range
	int c2 = CreateCondition("c2", 300); // above range
	int c3 = CreateCondition("", 0); // is this valid?
	// Conditions - the rest
	DestroyCondition(-1); // below range
	DestroyCondition(100001); // above range
	DestroyCondition(9999); // condition should be NULL
	// diff addr space (exec)


    Exit(0);
}
