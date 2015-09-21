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
#include <vector>
#include <iostream>
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

Semaphore * senatorSemaphore;
Lock * senatorLock;
Condition * senatorCondition;

int NUM_CUSTOMERS;
int NUM_SENATORS;
int NUM_CLERKS;
int customersFinished = 0;

bool senatorHere = false;

struct Customer {
    
    char * name;
    bool applicationFiled;
    bool pictureTaken;
    bool pictureFiled;
    bool passportCertified;
    bool passportGiven;
    bool cashierPaid;
    int money;
    static int totalCustomerMoney;
    
    bool isSenator;
    
    Customer(char * _name, bool senator)
    {
        name = _name;
        pictureTaken = false;
        pictureFiled = false;
        applicationFiled = false;
        cashierPaid = false;
        passportGiven = false;
        passportCertified = false;
		isSenator = senator;
		if (isSenator) {
			money = 100;
		} else {
        	money = 100 + (rand() % 4)*500;
        }
        Customer::totalCustomerMoney += money;
    }
};

int Customer::totalCustomerMoney = 0;

struct Clerk {
    
    char * name;
    int lineCount;
    int bribeLineCount;
    int state;
    Condition * lineCondition;
    Condition * bribeLineCondition;
    Condition * clerkCondition;
    
    Lock * breakLock;
    Condition * breakCondition;
    
    Lock * clerkLock;
    int clerkType;
    Customer * customer;
    int money;
    
    Clerk()
    {
        name=NULL;
        lineCount = 0;
        bribeLineCount = 0;
        state=0;
        lineCondition= new Condition(name);
        bribeLineCondition = new Condition(name);
        clerkCondition = new Condition(name);
        clerkLock = new Lock(name);
        
        breakLock = new Lock(name);
        breakCondition = new Condition(name);
        
        clerkType = 0;
        customer = NULL;
        money = 0;
    }
    
    Clerk(char * _name, int type)
    {
        name = _name;
        lineCount = 0;
        bribeLineCount = 0;
        state = AVAILABLE;
        lineCondition = new Condition(name);
        bribeLineCondition = new Condition(name);
        clerkCondition = new Condition(name);
        clerkLock = new Lock(name);
        
        breakLock = new Lock(name);
        breakCondition = new Condition(name);
        
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

std::vector<Customer*> customers;
std::vector<Clerk*> pictureClerks;
std::vector<Clerk*> applicationClerks;
std::vector<Clerk*> passportClerks;
std::vector<Clerk*> cashiers;

Manager * clerkManager;

//the customer thread is running inside this method
void pictureTransaction(Clerk * clerk, Customer * customer) {
    
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    
    //printf("%s has acquired the lock %s \n \n", customer->name, clerk->clerkLock->getName());
    
    //printf("%s is waiting for %s to take their picture\n \n", customer->name, clerk->name);
    clerk->clerkCondition->Signal(clerk->clerkLock);
    printf("%s has given SSN to %s\n",customer->name, clerk->name);
    
    while(!clerk->customer->pictureTaken) {
        
        clerk->clerkCondition->Wait(clerk->clerkLock);
        if((rand() % 10) == 0) {
            
            printf("%s does not like their picture from %s\n", customer->name, clerk->name);
            clerk->clerkCondition->Signal(clerk->clerkLock);
        } else {
            
            printf("%s does like thier picture from %s\n", customer->name,clerk->name);
            clerk->customer->pictureTaken = true;
            clerk->clerkCondition->Signal(clerk->clerkLock);
            clerk->clerkCondition->Wait(clerk->clerkLock);
        }
    }
    
    clerk->clerkLock->Release();
}

void pictureClerk(int myLine) {
    
    Clerk * me = pictureClerks[myLine];
    
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        
        //printf("There are still customers so %s isn't finished yet \n\n", me->name);
        pictureClerkLock->Acquire();
        
        // If there is a customer in line signal him to the counter
        if(me->bribeLineCount > 0) {
        	printf("%s has signalled a Customer to come to their counter\n", me->name);
            me->bribeLineCondition->Signal(pictureClerkLock);
            me->state = BUSY;
        } else if(me->lineCount > 0) {
            printf("%s has signalled a Customer to come to their counter\n", me->name);
            me->lineCondition->Signal(pictureClerkLock);
            me->state = BUSY;
        } else {
            
            printf("%s is going on break\n", me->name);
            pictureClerkLock->Release();
            me->breakLock->Acquire();
            me->state = ONBREAK;
            me->breakCondition->Wait(me->breakLock);
            printf("%s is coming off break\n", me->name);
            me->breakLock->Release();
            pictureClerkLock->Acquire();
            
            me->lineCondition->Signal(pictureClerkLock);
        }
        
        me->clerkLock->Acquire();
        pictureClerkLock->Release();
        
        //printf("%s is waiting for customer data \n \n", me->name);
        me->clerkCondition->Wait(me->clerkLock);
        printf("%s has received SSN from %s", me->name,me->customer->name);
        bool firstTime = true;
        while(!me->customer->pictureTaken) {
            
            if(!firstTime)
                printf("%s has been told that %s does not like their picture\n", me->name, me->customer->name);
            printf("%s has taken a picture of %s\n", me->name, me->customer->name);
            me->clerkCondition->Signal(me->clerkLock);
            me->clerkCondition->Wait(me->clerkLock);
            firstTime=false;
        }
        
        printf("%s has been told that %s does like their picture\n", me->name,me->customer->name);
        for(int i = 0; i < rand()%80+20;i++)
        {
            currentThread->Yield();
        }
        
        me->customer->pictureFiled = true;
        me->clerkCondition->Signal(me->clerkLock);
        me->clerkLock->Release();
    }
}


void applicationTransaction(Clerk * clerk, Customer * customer) {
    
    
    //printf("%s is about to perform an application transaction with %s \n \n", customer->name, clerk->name);
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    
    printf("%s has given SSN to %s\n", customer->name, clerk->name);
    clerk->clerkCondition->Signal(clerk->clerkLock);
    clerk->clerkCondition->Wait(clerk->clerkLock);
    
    //printf("%s has just gotten his application filed by %s \n \n",customer->name, clerk->name);
    
    clerk->clerkLock->Release();
}

void applicationClerk(int myLine) {
    
    Clerk * me = applicationClerks[myLine];
    
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        
        //printf("There are still customers so %s isn't finished yet \n\n", me->name);
        
        applicationClerkLock->Acquire();
        
        // If there is a customer in line signal him to the counter
         if(me->bribeLineCount > 0) {
        	printf("%s has signalled a customer to come to their counter\n", me->name);
            me->bribeLineCondition->Signal(applicationClerkLock);
            me->state = BUSY;
        } else if(me->lineCount > 0) {
            me->lineCondition->Signal(applicationClerkLock);
            me->state = BUSY;
            printf("%s has signalled a customer to come to their counter\n", me->name, me->lineCount);
        } else {
            
            printf("%s is going on break\n", me->name);
            
            applicationClerkLock->Release();
            me->breakLock->Acquire();
            me->state = ONBREAK;
            me->breakCondition->Wait(me->breakLock);
            printf("%s is coming off break\n",me->name);
            me->breakLock->Release();
            applicationClerkLock->Acquire();
            
            me->lineCondition->Signal(applicationClerkLock);
        }
        
        me->clerkLock->Acquire();
        applicationClerkLock->Release();
        
        //printf("%s is waiting for customer data \n\n", me->name);
        me->clerkCondition->Wait(me->clerkLock);
        
        printf("%s has received SSN from %s \n", me->name, me->customer->name);
        for(int i =0;i<rand() % 80 + 20;i++)
        {
            currentThread->Yield();
        }
        me->customer->applicationFiled = true;
        
        printf("%s has recoreded a completed application for %s\n", me->name, me->customer->name);
        me->clerkCondition->Signal(me->clerkLock);
        
        //adding this based on Crowley's code
        //me->state=ONBREAK;
        //me->clerkCondition->Wait(me->clerkLock);
        
        me->clerkLock->Release();
    }    
}

void passportTransaction(Clerk * clerk, Customer * customer) {
    
    //printf("%s is about to perform a passport transaction with %s \n \n", customer->name, clerk->name);
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    
    printf("%s has given SSN to %s\n", customer->name, clerk->name);
    clerk->clerkCondition->Signal(clerk->clerkLock);
    clerk->clerkCondition->Wait(clerk->clerkLock);
    
    //printf("%s has just gotten his passport filed by %s \n\n",customer->name, clerk->name);
    
    clerk->clerkLock->Release();
}

void cashierTransaction(Clerk * clerk, Customer * customer) {
    
    //printf("%s is about to perform a cashier transaction with %s \n\n", customer->name, clerk->name);
    clerk->clerkLock->Acquire();
    clerk->customer = customer;
    
    clerk->clerkCondition->Signal(clerk->clerkLock);
    
    customer->cashierPaid = true;
    printf("%s has given SSN to %s\n", customer->name, clerk->name);
    
    clerk->clerkCondition->Wait(clerk->clerkLock);
    
    customersFinished++;
    printf("%s is leaving the passport office\n", customer->name);
    
    clerk->clerkLock->Release();
}

int getInShortestLine(Customer * customer, std::vector<Clerk*> clerkToVisit, Lock * clerkLock) {

    clerkLock->Acquire();
    int myLine = -1;
    int shortestLineSize = NUM_CUSTOMERS + NUM_SENATORS; 
    bool foundLine = false;
    bool bribed = false;
    
    bool allOnBreak = true;
    for(int i = 0 ; i < NUM_CLERKS; i++)
    {
        if(clerkToVisit[i]->state == BUSY || clerkToVisit[i]->state == AVAILABLE)
            allOnBreak=false;
    }
    
    if(!allOnBreak) {
    
        while(!foundLine) {
            if (customer->money >= 600 && clerkToVisit[0]->clerkType != CASHIER) { // because you need 500 to bribe and 100 to pay cashier
                for(int i = 0; i < NUM_CLERKS; i++) {
                    if((clerkToVisit[i]->bribeLineCount < shortestLineSize) && (clerkToVisit[i]->state != ONBREAK)) {
                        myLine = i;
                        shortestLineSize = clerkToVisit[i]->bribeLineCount;
                        foundLine = true;
                        bribed = true;
                    }
                }
            } else {
                for(int i = 0; i < NUM_CLERKS; i++) {
                    if((clerkToVisit[i]->lineCount + clerkToVisit[i]->bribeLineCount < shortestLineSize) && (clerkToVisit[i]->state != ONBREAK)) {
                        myLine = i;
                        shortestLineSize = clerkToVisit[i]->lineCount + clerkToVisit[i]->bribeLineCount;
                        foundLine = true;
                    }
                }
            }
        }
        
    }
    else {
        if (customer->money >= 600 && clerkToVisit[0]->clerkType != CASHIER) {
            
            myLine = 0;
            shortestLineSize = clerkToVisit[0]->bribeLineCount;
            foundLine = true;
            bribed = true;
        }
        else {
            
            myLine = 0;
            shortestLineSize = clerkToVisit[0]->lineCount + clerkToVisit[0]->bribeLineCount;
            foundLine = true;
        }
    }
    //printf("The shortest line found for %s was line %d \n\n", customer->name, myLine);
    
    if(clerkToVisit[myLine]->state == BUSY || clerkToVisit[myLine]->state == ONBREAK) {

    	if (bribed) {
    		printf("%s has gotten in bribe line for %s\n", customer->name, clerkToVisit[0]->name);
        	clerkToVisit[myLine]->bribeLineCount++;
        	clerkToVisit[myLine]->bribeLineCondition->Wait(clerkLock);
        	clerkToVisit[myLine]->bribeLineCount--;
    	} else {
    		// getting in regular line
        	printf("%s has gotten in regular line for %s\n", customer->name,clerkToVisit[0]->name);
            clerkToVisit[myLine]->lineCount++;
        	clerkToVisit[myLine]->lineCondition->Wait(clerkLock);
        	clerkToVisit[myLine]->lineCount--;
    	}
    	
    }

    if (bribed) {
    		customer->money -= 500;
    		//printf("%s had $%d but he bribed %s so he now has %d \n\n", customer->name, customer->money+500,clerkToVisit[myLine]->name, customer->money);
    		clerkToVisit[myLine]->money += 500;
    }
    
    clerkToVisit[myLine]->state = BUSY;
    clerkLock->Release();
    
    //printf("We are informing %s that his line # is %d \n\n",customer->name, myLine);
    return myLine;
}

void customer(int customerNumber) {

    Customer * me = customers[customerNumber];
    //printf("Printing Customer Name: %s \n\n", me->name);

    //This will prevent any new customers from entering the store while the senator is still there.
    if(me->isSenator)
    {
        //printf("%s just entered the office, we are making all customers not currently being serviced leave the office \n\n", me->name);
        senatorSemaphore->P();
        //senatorLock->Acquire();
        senatorHere = true;
    }
    
    int randomNum = rand() % 2;
    
    if(randomNum == 0)
    {
        //printf("%s is going to Picture Clerk first \n\n", me->name);
        int myLine = getInShortestLine(me, pictureClerks, pictureClerkLock);
        pictureTransaction(pictureClerks[myLine], me);
        
         //if there is a senator in the office then we still need to wait

        if(senatorHere && !(me->isSenator))
        {
            printf("%s is going outside the Passport Office because there is a Senator present\n", me->name);
            senatorLock->Acquire();
            senatorCondition->Wait(senatorLock);
            senatorLock->Release();
        }

        //printf("%s is going to the Application Clerk second \n \n", me->name);
        myLine = getInShortestLine(me, applicationClerks, applicationClerkLock);  
        applicationTransaction(applicationClerks[myLine], me);
    }
    else // randomNum == 1
    {
        //printf("%s is going to the Application Clerk first \n\n", me->name);
        int myLine = getInShortestLine(me, applicationClerks, applicationClerkLock);
        applicationTransaction(applicationClerks[myLine], me);
        
   		if(senatorHere && !(me->isSenator))
        {
            printf("%s is going outside the Passport Office because there is a Senator present\n", me->name);
            senatorLock->Acquire();
            senatorCondition->Wait(senatorLock);
            senatorLock->Release();
        }

        //printf("%s is going to the picture clerk second \n\n", me->name);
        myLine = getInShortestLine(me, pictureClerks, pictureClerkLock);
        pictureTransaction(pictureClerks[myLine], me);
    }

    //printf("%s has reached the passport clerk, attempting to certify his passport now \n\n", me->name);
    while(!me->passportCertified) {
                
    	if(senatorHere && !(me->isSenator))
        {
            printf("%s is going outside the Passport Office because there is a Senator present\n", me->name);
            senatorLock->Acquire();
            senatorCondition->Wait(senatorLock);
            senatorLock->Release();
        }

        if(!me->pictureFiled || !me->applicationFiled) {
            //printf("%s does not have both their application and picture completed\n", me->name);
            for(int i = 0; i < rand() % 900 + 100; i++) {
                currentThread->Yield();
            }
        } else {
			int myLine = getInShortestLine(me, passportClerks, passportClerkLock);
            passportTransaction(passportClerks[myLine], me);
        }
    }
    
    //printf("%s has gotten his passport certified, attempting to pay now \n\n", me->name);
    while(!me->cashierPaid) {

		if(senatorHere && !(me->isSenator))
        {
            printf("%s is going outside the Passport Office because there is a Senator present\n", me->name);
            senatorLock->Acquire();
            senatorCondition->Wait(senatorLock);
            senatorLock->Release();
        }

        if(!me->passportCertified) {
            for(int i = 0; i < rand() % 900 + 100; i++) {
                currentThread->Yield();
            }
        } else {
			int myLine = getInShortestLine(me, cashiers, cashierLock);
            cashierTransaction(cashiers[myLine], me);
        }
    }
    
    if(me->isSenator)
    {
        //printf("%s has just finished using the office, we are broadcasting for all customers to come back into the office \n\n",me->name);
        senatorHere = false;
        //senatorLock->Release();
        senatorCondition->Broadcast(senatorLock);
        senatorSemaphore->V();
    }
}

void passportClerk(int myLine) {
    
    Clerk * me = passportClerks[myLine];
    
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        //printf("%s has %d in his bribe line and %d in reg line \n\n", me->name, me->bribeLineCount, me->lineCount);
        passportClerkLock->Acquire();
        
        // If there is a customer in line signal him to the counter
         if(me->bribeLineCount > 0) {
        	printf("%s has signalled a customer to come to their counter\n", me->name);
            me->bribeLineCondition->Signal(passportClerkLock);
            me->state = BUSY;
        } else if(me->lineCount > 0) {
            printf("%s has signalled a customer to come to their counter\n", me->name, me->lineCount);
            me->lineCondition->Signal(passportClerkLock);
            me->state = BUSY;
        } else {
            
            printf("%s is going on break\n", me->name);
            passportClerkLock->Release();
            me->breakLock->Acquire();
            me->state = ONBREAK;
            me->breakCondition->Wait(me->breakLock);
            printf("%s is coming off break\n", me->name);
            me->breakLock->Release();
            passportClerkLock->Acquire();
            
            me->lineCondition->Signal(passportClerkLock);
        }

        me->clerkLock->Acquire();
        passportClerkLock->Release();

        //printf("%s is waiting for customer data \n\n", me->name);
        me->clerkCondition->Wait(me->clerkLock);

        printf("%s has received SSN from %s\n", me->name, me->customer->name);
        printf("%s has determined that %s does have both their application and picture completed\n",me->name,me->customer->name);
        for(int i = 0; i < rand()% 80 + 20; i++) {
            currentThread->Yield();
        }
        me->customer->passportCertified = true;
        printf("%s has recorded %s passport documentation\n", me->name, me->customer->name);
        me->clerkCondition->Signal(me->clerkLock);
        me->clerkLock->Release();
    }
}


void cashier (int myLine) {
    Clerk * me = cashiers[myLine];
    
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
    	cashierLock->Acquire();

    	if (me->lineCount > 0) {
            
            printf("%s has signalled a customer to come to their counter\n", me->name);
    		me->lineCondition->Signal(cashierLock);
    		me->state = BUSY;
    	} else {
            printf("%s is going on break\n", me->name);
            cashierLock->Release();
            me->breakLock->Acquire();
            me->state = ONBREAK;
            me->breakCondition->Wait(me->breakLock);
            
            printf("%s is coming off break\n");
            me->breakLock->Release();
            cashierLock->Acquire();
            
            me->lineCondition->Signal(cashierLock);
    	}

    	me->clerkLock->Acquire();
    	cashierLock->Release();

        
        //printf("%s is waiting for a customer to come pay \n\n", me->name);
    	// wait for customer to pay
    	me->clerkCondition->Wait(me->clerkLock);

        printf("%s has received SSN from %s\n", me->name,me->customer->name);
        printf("%s has verified that %s has been certified by a PassportClerk\n", me->name, me->customer->name);
    	// taking payment
    	me->customer->money -= 100;
    	me->money += 100;
        printf("%s has received the $100 from %s after certification\n\n", me->name, me->customer->name);
        
    	me->customer->passportGiven = true;
    	me->clerkCondition->Signal(me->clerkLock);
        printf("%s has provided %s their completed passport\n",me->name,me->customer->name);
        printf("%s has recorded that %s has been given their completed passport\n",me->name,me->customer->name);
        
        //adding this based on Crowley's code
        //me->state=ONBREAK;
        //me->clerkCondition->Wait(me->clerkLock);
        
    	me->clerkLock->Release();
    } 
}

void manager() {
    
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        
        //printf("There are still customers so the manager will now make sure no clerks are on break with 3 or more people in their line \n\n");
        
        
        bool signalPictureClerk=false;
        bool signalAppClerk=false;
        bool signalPassportClerk=false;
        bool signalCashier=false;
        
        bool pictureClerksAllOnBreak = true;
        bool applicationClerksAllOnBreak = true;
        bool passportClerksAllOnBreak = true;
        bool cashiersAllOnBreak = true;
        
        for(int i = 0; i< NUM_CLERKS;i ++)
        {
            if(pictureClerks[i]->lineCount>=3)
                signalPictureClerk=true;
            if(applicationClerks[i]->lineCount>=3)
                signalAppClerk=true;
            if(passportClerks[i]->lineCount>=3)
                signalPassportClerk=true;
            if(cashiers[i]->lineCount>=3)
                signalCashier=true;
            if(pictureClerks[i]->state == (BUSY || AVAILABLE))
                pictureClerksAllOnBreak = false;
            if(applicationClerks[i]->state == (BUSY || AVAILABLE))
                applicationClerksAllOnBreak = false;
            if(passportClerks[i]->state == (BUSY || AVAILABLE))
                passportClerksAllOnBreak = false;
            if(cashiers[i]->state == (BUSY || AVAILABLE))
                cashiersAllOnBreak = false;
        }
        
        if(pictureClerksAllOnBreak)
        {
            printf("Manager has woken up a PictureClerk\n");
            pictureClerkLock->Acquire();
            pictureClerks[0]->state = AVAILABLE;
            pictureClerks[0]->breakCondition->Signal(pictureClerks[0]->breakLock);
            pictureClerkLock->Release();
            pictureClerksAllOnBreak=false;
        }
        if(applicationClerksAllOnBreak)
        {
            printf("Manager has woken up an ApplicationClerk\n");
            applicationClerkLock->Acquire();
            applicationClerks[0]->state = AVAILABLE;
            applicationClerks[0]->breakCondition->Signal(applicationClerks[0]->breakLock);
            applicationClerkLock->Release();
            applicationClerksAllOnBreak=false;
        }
        if(passportClerksAllOnBreak)
        {
            printf("Manager has woken up a PassportClerk\n");
            passportClerkLock->Acquire();
            passportClerks[0]->state = AVAILABLE;
            passportClerks[0]->breakCondition->Signal(passportClerks[0]->breakLock);
            passportClerkLock->Release();
            passportClerksAllOnBreak=false;
        }
        if(cashiersAllOnBreak)
        {
            printf("Manager has woken up a Cashier\n");
            cashierLock->Acquire();
            cashiers[0]->state = AVAILABLE;
            cashiers[0]->breakCondition->Signal(cashiers[0]->breakLock);
            cashierLock->Release();
            cashiersAllOnBreak = false;
        }
        
        for(int i = 0; i < NUM_CLERKS; i++)
        {
            if(signalPictureClerk && pictureClerks[i]->state == ONBREAK)
            {
                printf("Manager has woken up a PictureClerk\n");
                pictureClerkLock->Acquire();
                pictureClerks[i]->state = AVAILABLE;
                pictureClerks[i]->breakCondition->Signal(pictureClerks[i]->breakLock);
                pictureClerkLock->Release();
                signalPictureClerk=false;
            }
            if(signalAppClerk && applicationClerks[i]->state == ONBREAK)
            {
                printf("Manager has woken up an ApplicationClerk\n");
                applicationClerkLock->Acquire();
                applicationClerks[i]->state = AVAILABLE;
                applicationClerks[i]->breakCondition->Signal(applicationClerks[i]->breakLock);
                applicationClerkLock->Release();
                signalAppClerk=false;
                
            }
            if(signalPassportClerk && passportClerks[i]->state == ONBREAK)
            {
                printf("Manager has woken up a PassportClerk\n");
                passportClerkLock->Acquire();
                passportClerks[i]->state = AVAILABLE;
                passportClerks[i]->breakCondition->Signal(passportClerks[i]->breakLock);
                passportClerkLock->Release();
                signalPassportClerk=false;
            }
            if(signalCashier && cashiers[i]->state == ONBREAK)
            {
                printf("Manager has woken up a Cashier\n");
                cashierLock->Acquire();
                cashiers[i]->state = AVAILABLE;
                cashiers[i]->breakCondition->Signal(cashiers[i]->breakLock);
                cashierLock->Release();
                signalCashier = false;
            }
        }
        
        //printf("Manager waiting a bit to print the revenue \n\n");
        
        for(int i =0;i<400;i++)
        {
            currentThread->Yield();
        }
        
        //printf("Manager will print the revenue now: \n\n\n");

        int pictureRevenue = 0;
        int applicationRevenue = 0;
        int passportRevenue = 0;
        int cashierRevenue = 0;

        for (int i = 0; i < NUM_CLERKS; i++)
        {
        	pictureRevenue += pictureClerks[i]->money;
        	applicationRevenue += applicationClerks[i]->money;
        	passportRevenue += passportClerks[i]->money;
        	cashierRevenue += cashiers[i]->money;  		
        }
        
        printf("Manager has counted a total of $%d for PictureClerks\n", pictureRevenue);
        printf("Manager has counted a total of $%d for ApplicationClerks\n", applicationRevenue);
        printf("Manager has counted a total of $%d for PassportClerks\n", passportRevenue);
        printf("Manager has counted a total of $%d for Cashiers\n", cashierRevenue);
        printf("Manager has counted a total of $%d for the passport office\n", (pictureRevenue+applicationRevenue+passportRevenue+cashierRevenue));
        //printf("Total money walking in the door: %d \n\n", Customer::totalCustomerMoney);
    }
    // TODO:deallocation and terminate
}

int isValidNumber(char* number, int limit) {
    int num;
    try 
    {
        num = atoi(number);
        if (num < 1 || num > limit) {
            num = -1;
        }
    } catch (int e) {
        num = -1;
    }
    return num;
}

void launchPassportOffice() 
{
    Thread *t;
    char *name;
    int i;

    bool valid = false;
    while (!valid) 
    {
        char* z = new char[20];
        std::cout << "Please enter the number of customers (1-50): ";
        std::cin >> z;  

        int num_cust = isValidNumber(z, 50);

        if (num_cust != -1){
            valid = true;
            NUM_CUSTOMERS = num_cust;
        } else {
            printf("That was an invalid number. \n");
        }
    }

    valid = false;
    while (!valid) 
    {
        char* z = new char[20];
        std::cout << "Please enter the number of clerks and cashiers (1-5): ";
        std::cin >> z;  

        int num_clerk = isValidNumber(z, 5);

        if (num_clerk != -1){
            valid = true;
            NUM_CLERKS = num_clerk;
        } else {
            printf("That was an invalid number. \n");
        }
    }

    valid = false;
    while (!valid) 
    {
        char* z = new char[20];
        std::cout << "Please enter the number of senators (1-10): ";
        std::cin >> z;  

        int num_sen = isValidNumber(z, 10);

        if (num_sen != -1){
            valid = true;
            NUM_SENATORS = num_sen;
        } else {
            printf("That was an invalid number. \n");
        }
    }
    
    printf("Number of Customers = %d\n", NUM_CUSTOMERS);
    printf("Number of ApplicationClerks = %d\n\n", NUM_CLERKS);
    printf("Number of PictureClerks = %d\n\n", NUM_CLERKS);
    printf("Number of PassportClerks = %d\n\n", NUM_CLERKS);
    printf("Number of Cashiers = %d\n\n", NUM_CLERKS);
    printf("Number of Senators = %d\n\n", NUM_SENATORS);

    pictureClerkLock = new Lock("Picture Lock");
    applicationClerkLock = new Lock("Application Lock");
    passportClerkLock = new Lock("Passport Lock");
    cashierLock = new Lock("Cashier Lock");
    
    senatorSemaphore = new Semaphore("Senator Semaphore", NUM_SENATORS);
    senatorLock = new Lock("Senator Lock");
    senatorCondition = new Condition("Senator Condition");
   
    clerkManager = new Manager("Manager 0");
    t = new Thread("Manager 0");
    t->Fork((VoidFunctionPtr)manager, 0);

    for( i = 0; i < NUM_CLERKS; i ++)
    {
        name = new char [20];
        sprintf(name,"Picture Clerk %d",i);
        pictureClerks.push_back(new Clerk(name, PICTURECLERK));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)pictureClerk, i);
        
        name = new char [20];
        sprintf(name,"Application Clerk %d",i);
        applicationClerks.push_back(new Clerk(name, APPLICATIONCLERK));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)applicationClerk, i);

        
        name = new char [20];
        sprintf(name,"Passport Clerk %d",i);
        passportClerks.push_back(new Clerk(name, PASSPORTCLERK));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)passportClerk, i);
        
        name = new char [20];
        sprintf(name,"Cashier %d",i);
        cashiers.push_back(new Clerk(name, CASHIER));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)cashier, i);
    }
    
    for( i = 0;i < NUM_CUSTOMERS; i++)
    {
        name = new char [20];
        sprintf(name,"Customer %d", i);
        customers.push_back(new Customer(name, false));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)customer, i);
    }
    
    for(i =0;i<NUM_SENATORS;i++)
    {
        name = new char [20];
        sprintf(name,"Senator %d", i);
        customers.push_back(new Customer(name,true));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)customer,NUM_CUSTOMERS + i);
    }
}

// Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time
void testOne() 
{
    Thread *t;
    char *name;
    int i;

    NUM_CUSTOMERS = 5;
    NUM_CLERKS = 2;

    pictureClerkLock = new Lock("Picture Lock");
    applicationClerkLock = new Lock("Application Lock");
    passportClerkLock = new Lock("Passport Lock");
    cashierLock = new Lock("Cashier Lock");
      
    clerkManager = new Manager("Manager 0");
    t = new Thread("Manager 0");
    t->Fork((VoidFunctionPtr)manager, 0);

    for( i = 0; i < NUM_CLERKS; i ++)
    {
        name = new char [20];
        sprintf(name,"Picture Clerk %d",i);
        pictureClerks.push_back(new Clerk(name, PICTURECLERK));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)pictureClerk, i);
        
        name = new char [20];
        sprintf(name,"Application Clerk %d",i);
        applicationClerks.push_back(new Clerk(name, APPLICATIONCLERK));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)applicationClerk, i);

        
        name = new char [20];
        sprintf(name,"Passport Clerk %d",i);
        passportClerks.push_back(new Clerk(name, PASSPORTCLERK));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)passportClerk, i);
        
        name = new char [20];
        sprintf(name,"Cashier %d",i);
        cashiers.push_back(new Clerk(name, CASHIER));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)cashier, i);
    }
    
    for( i = 0;i < NUM_CUSTOMERS; i++)
    {
        name = new char [20];
        sprintf(name,"Customer %d", i);
        customers.push_back(new Customer(name, false));
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)customer, i);
    }
}

// Managers only read one from one Clerk's total money received, at a time.
void testTwo()
{

}


// Customers do not leave until they are given their passport by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area
void testThree()
{

}    

// Clerks go on break when they have no one waiting in their line
void testFour()
{

}

// Managers get Clerks off their break when lines get too long
void testFive()
{

}

// Total sales never suffers from a race condition
void testSix()
{

}

// The behavior of Customers is proper when Senators arrive. This is before, during, and after. 
void testSeven()
{

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
    
    /*
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
     
    */
    
    
    //Part 2
    printf("\n \n \n Starting Part 2 \n \n \n");

    // launchPassportOffice();

    testOne();
    // testTwo();
    // testThree();
    // testFour();
    // testFive();
    // testSix();
    // testSeven();
}
#endif
