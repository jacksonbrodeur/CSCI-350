// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
/*
#include "copyright.h"
#include "system.h"
*/


#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}


//	Simple test cases for the threads assignment.
//


// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
// lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
// lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
// lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
// done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
    
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
            t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
            t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {
    
    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock
    
    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Acquire();
    
    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
            t1_l1.getName());
    for (int i = 0; i < 10; i++)
        ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
            t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {
    
    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock
    
    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
        printf("%s: Trying to release Lock %s\n",currentThread->getName(),
               t1_l1.getName());
        t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
// done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
           t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    
    printf("Printing cvWaitQueue address inside test function for t2_t2, %d \n \n", t2_c1.GetQueueAddress());
    
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
// done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {
    
    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ )
        t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
           t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
// done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {
    
    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ )
        t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
           t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after

Semaphore t5_done("t5_done",0);
// t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
           t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
           t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
           t5_l1.getName());
    t5_l1.Release();
    
    t5_done.V();
}

static const int AVAILABLE = 0;
static const int BUSY = 1;
static const int ONBREAK = 2;

static const int PICTURECLERK = 0;
static const int APPLICATIONCLERK = 1;
static const int PASSPORTCLERK = 2;
static const int CASHIER = 3;

Lock * applicationClerkLock;
Lock * pictureClerkLock;
Lock * passportClerkLock;
Lock * cashierLock;

Customer * customers[10];
Clerk * pictureClerks[5];
Clerk * applicationClerks[5];
Clerk * passportClerks[5];
Clerk * cashiers[5];

struct Customer {
    
    char * name;
    bool applicationFiled;
    bool pictureTaken;
    bool pictureFiled;
    bool passportFiled;
    bool cashierPaid;
    int money;
    
    Customer(char * _name)
    {
        name = _name;
        applicationFiled = false;
        pictureTaken = false;
        passportFiled = false;
        cashierPaid = false;
        pictureFiled = false;
        money = 100 + (rand() % 4)*500;
    }
};

struct Clerk {
    
    char * name;
    int lineCount;
    int state;
    Condition * lineCondition;
    Condition * clerkCondition;
    Lock * clerkLock;
    int clerkType;
    Customer * customer;
    int money;
    
    PictureClerk(char * _name, int type)
    {
        name = _name;
        lineCount = 0;
        state = AVAILABLE;
        lineCondition = new Condition("%s's line condition variable", name);
        clerkCondition = new Condition("%s's clerk condition variable", name);
        clerkLock = new Lock("%'s lock");
        clerkType = type;
        money = 0;
    }
};


struct Manager {
    
    char * name;

    Manager(char * _name)
    {
        name = _name;
    }
    
};

struct Senator {
    
    char * name;

    Senator(char * _name)
    {
        name = _name;
    }
};

int getInShortestLine(Clerk * clerkToVisit[], Lock * clerkLock) {

    clerkLock->Acquire();
    int myLine = -1;
    int shortestLineSize = 10000; //change to the number of max customers
    bool foundLine = false;
    while(!foundLine) {
        for(int i=0;i<5;i++)
        {
            if(clerkToVisit[i]->lineCount<shortestLineSize && clerkToVisit[i]->state != ONBREAK)
            {
                myLine = i;
                shortestLineSize = clerkToVisit[i]->lineCount;
                foundLine = true;
            }
            
        }
    }
    
    if (clerkToVisit[myLine]->state == BUSY)
    {
        clerkToVisit[myLine]->lineCount++;
        clerkToVisit[myLine]->lineCondition->Wait(clerkLock);
        clerkToVisit[myLine]->lineCount--;
    }
    clerkToVisit[myLine]->state=BUSY;
    clerkLock->Release();
    return myLine;
}

void customer(int customerNumber) {

    Customer * me = customers[customerNumber];
    printf("Printing Customer Number: %d\n", customerNumber);

    int randomNum = rand() % 2;
    
    if(randomNum == 0)
    {
        printf("Going to Picture Clerk first \n \n");
        getInShortestLine(pictureClerks, pictureClerkLock);
        //Do Picture Clerk Stuff
        getInShortestLine(applicationClerks, applicationClerkLock);
        //Do Application Clerk Stuff
    }
    else // randomNum == 1
    {
        printf("Going to Application Clerk first \n \n");
        getInShortestLine(applicationClerks, applicationClerkLock);
        //Do Application Clerk Stuff
        getInShortestLine(pictureClerks, pictureClerkLock);
        //Do Picture Clerk Stuff
    }

    while(!me->passportFiled) {
        int myLine = getInShortestLine(passportClerks, passportClerkLock);
        if(!me->pictureFiled || !me->applicationFiled) {
            //Penalize me
            passportClerks[myLine]->state = AVAILABLE;
            int penalty = rand() % 900 + 100;
            for(int i = 0; i < penalty; i++) {
                currentThread->Yield();
            }
        } else {
            //Call passport transaction
        }
    }
    
    while(!me->cashierPaid) {
        int myLine = getInShortestLine(cashiers, cashierLock);
        if(!me->passportFiled) {
            //Penalize me
            cashier[myLine]->state = AVAILABLE;
            int penalty = rand() % 900 + 100;
            for(int i = 0; i < penalty; i++) {
                currentThread->Yield();
            }
        } else {
            //Call cashier transaction
        }
    }
   

    
}

void pictureClerk(int myLine) {
    Clerk * me = pictureClerks[myLine];
    while(true) {
        pictureClerkLock->Acquire();
        //TODO: Bribes

        // If there is a customer in line 
        // signal him to the counter
        if(me->lineCount > 0) {
            me->lineCondition->Signal(pictureClerkLock);
            me->state = BUSY;
        } else {
            me->state = AVAILABLE;
        }

        me->clerkLock->Acquire();
        pictureClerkLock->Release();

        //Wait for customer data
        me->clerkCondition->Wait(me->clerkLock);

        //Do your job, take the picture
        while(!me->customer->pictureTaken) {
            //Take the damn customer's picture
            me->clerkCondition->Signal(me->clerkLock);
            //Accept the freaking picture, customer
            me->clerkCondition->Wait(me->clerkLock);
        }

        //Yield for random amount

        me->clerkCondition->Signal(me->clerkLock);
        //All done with this bastard
        me->clerkLock->Release();
    }
}

void applicationClerk(int myLine) {
    Clerk * me = applicationClerks[myLine];
    while(true) {
        applicationClerkLock->Acquire();
        //TODO: Bribes

        // If there is a customer in line 
        // signal him to the counter
        if(me->lineCount > 0) {
            me->lineCondition->Signal(applicationClerkLock);
            me->state = BUSY;
        } else {
            me->state = AVAILABLE;
        }

        me->clerkLock->Acquire();
        applicationClerkLock->Release();

        //Wait for customer data
        me->clerkCondition->Wait(me->clerkLock);

        //Do your job, accept application

        yieldTime = rand() % 80 + 20;
        for(int i =0;i<yieldTime;i++)
        {
        	currentThread->Yield();
        }

        me->customer->applicationFiled = true;

        me->clerkCondition->Signal(me->clerkLock);
        //All done with this bastard
        me->clerkLock->Release();
    }    
}

void passportClerk(int myLine) {
    Clerk * me = passportClerks[myLine];
    while(true) {
        passportClerkLock->Acquire();
        //TODO: Bribes

        // If there is a customer in line 
        // signal him to the counter
        if(me->lineCount > 0) {
            me->lineCondition->Signal(pictureClerkLock);
            me->state = BUSY;
        } else {
            me->state = AVAILABLE;
        }

        me->clerkLock->Acquire();
        passportClerkLock->Release();

        me->clerkCondition->Wait(me->clerkLock);

        //Yield for random amount
        int delay = rand()% 80 + 20;
        for(int i = 0; i < delay; i++) {
            currentThread->Yield();
        }
        me->customer->passportFiled = true;
        me->clerkCondition->Signal(me->clerkLock);
        //Done
        me->clerkLock->Release();
    }
}

void passportTransaction(Clerk * clerk, Customer * customer) {
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    clerk->clerkCondition->Signal(clerk->clerkLock);
    clerk->clerkCondition->Wait(clerk->clerkLock);

    clerk->clerkLock->Release();
}

void cashier (int myLine) {
    Clerk * me = cashiers[myLine];
    while(true) {
    	cashierLock->Acquire();

    	// TODO: bribes
    	if (me->lineCount > 0) {
    		me->lineCondition->Signal(cashierLock);
    		me->state = BUSY;
    	} else {
    		me->state = AVAILABLE;
    	}

    	me->clerkLock->Acquire();
    	cashierLock->Release();

    	// wait for customer to pay
    	me->clerkCondition->Wait(me->clerkLock);

    	// taking payment
    	me->customer->money -= 100;
    	me->money += 100;

    	me->customer->passportFiled = true;
    	me->clerkCondition->Signal(me->clerkLock);
    	me->clerkLock->Release();
    } 
}

void cashierTransaction(Clerk * clerk, Customer * customer) {
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    //Signal that I have paid
    clerk->clerkCondition->Signal(clerk->clerkLock);
    //Wait for Cashier to file my passport
    clerk->clerkCondition->Wait(clerk->clerkLock);
    clerk->clerkLock->Release();
}

void manager() {
    
    
}

void senator() {
    
}

void pictureTransaction(Clerk * clerk, Customer * customer) {
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    // taking picture
    clerk->clerkCondition->Signal(clerk->clerkLock);



    while(!clerk->customer->pictureTaken) {
        
        if((rand() % 10) == 0) { // they don't like it
            clerk->clerkCondition->Signal(clerk->clerkLock);
        } else { // they like it
            clerk->customer->pictureTaken = true;
            clerk->clerkCondition->Signal(clerk->clerkLock);
        }
    }

    // clerk is filing application
    clerk->clerkCondition->Wait(clerk->clerkLock);
    // application transaction done
    clerk->clerkLock->Release();
}

void applicationTransaction(Clerk * clerk, Customer * customer) {
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    // passing application
    clerk->clerkCondition->Signal(clerk->clerkLock);
    // clerk is filing application
    clerk->clerkCondition->Wait(clerk->clerkLock);
    // application transaction done
    clerk->clerkLock->Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1
    
    printf("Starting Test 1\n");
    
    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);
    
    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);
    
    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);
    
    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
        t1_done.P();
    
    // Test 2
    
    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");
    
    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);
    
    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);
    
    // Wait for Test 2 to complete
    t2_done.P();
    
    // Test 3
    
    printf("Starting Test 3\n");
    
    for (  i = 0 ; i < 5 ; i++ ) {
        name = new char [20];
        sprintf(name,"t3_waiter%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);
    
    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
        t3_done.P();
    
    // Test 4
    
    printf("Starting Test 4\n");
    
    for (  i = 0 ; i < 5 ; i++ ) {
        name = new char [20];
        sprintf(name,"t4_waiter%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);
    
    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
        t4_done.P();
    
    // Test 5
    
    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");
    
    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);
    
    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);
    
    t5_done.P();
    //Part 2
    
    printf(" \n \n \n \n \n Starting Part 2 \n \n \n");
    
    pictureLock = new Lock("Picture Lock");
    applicationLock = new Lock("Application Lock");
    passportLock = new Lock("Passport Lock");
    cashierLock = new Lock("Cashier Lock");
   
    
    for(int i = 0;i < 10; i++)
    {
        name = new char [20];
        sprintf(name,"Customer %d",i);
        customers[i] = new Customer(name);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr), i+1);
    }
    
    for(int i = 0; i < 5; i ++)
    {
        name = new char [20];
        sprintf(name,"Picture Clerk %d",i);
        pictureClerks[i] = new Clerk(name, PICTURECLERK);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)pictureClerk, i);
        
        name = new char [20];
        sprintf(name,"Application Clerk %d",i);
        applicationClerks[i] = new Clerk(name, APPLICATIONCLERK);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)applicationClerk, i);

        
        name = new char [20];
        sprintf(name,"Passport Clerk %d",i);
        passportClerks[i] = new Clerk(name, PASSPORTCLERK);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)passportClerk, i);
        
        name = new char [20];
        sprintf(name,"Cashier %d",i);
        cashiers[i] = new Clerk(name, CASHIER);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)clerk, i);
    }
}
#endif
