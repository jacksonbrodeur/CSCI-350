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

int l1, l2, c1, c2, lock1, cond1;
int main()
{

	/* TEST CASES
		Need to test: (for all calls except Create)
			- below and above range
			- null object at index
			- different addr space
	
		Only in create lock/condition do we check anything else - MAXFILENAME (=256) & bad vaddr
	*/

	/* Locks - Create */
	l1 = CreateLock("l1", -1); /* below range */
	l2 = CreateLock("l2", 300); /* above range */
	/* code to test bad vaddr */ /* can we test bad vaddr? */

	/* Conditions - Create */
	c1 = CreateCondition("c1", -1); /* below range */
	c2 = CreateCondition("c2", 300); /* above range */ 
    
	/* Conditions - the rest */
	DestroyCondition(-1); /* below range */
	DestroyCondition(1001); /* above range */
	DestroyCondition(0); /* condition should be NULL */

	/* Locks - the rest */
	DestroyLock(-1); /* below range */
	DestroyLock(1001); /* above range */
	DestroyLock(0); /* lock should be NULL */

	cond1 = CreateCondition("cond1", 5);
	lock1 = CreateLock("lock1", 5);

	Exec("../test/diff_proc", 17); /* diff addr space (exec) */

    Exit(0);
}
