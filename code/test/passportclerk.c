#include "syscall.h"
#include "setup.h"

#define NUM_CUSTOMERS 20
#define NUM_SENATORS 10



main()
{
    int myLine;
    int i;
    int counterLock = GetSyscall(/*counterLock index*/);

    int lineCount;
    int bribeLineCount;
    int state;
    int lineCondition;
    int bribeLineCondition;
    int clerkCondition;
    
    int breakLock;
    int breakCondition;
    
    int clerkLock;
    int clerkType;
    Customer * customer;
    int money;
    
    Acquire(counterLock);
    myLine = CreateSyscall(/*numPassportClerks*/);
    SetSyscall(/*numPassportClerks++*/);
    me = &passportClerks[myLine];
    Release(counterLock);
    
    /* On duty while there are still customers who haven't completed process */
    while(GetSyscall(/*customersFinished*/) < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(passportClerkLock);
        
        /* If there is a customer in line signal him to the counter */
        if(GetSyscall(/*me->bribeLineCount*/) > 0) { /* Check bribe line first... */
            Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
            Signal(me->bribeLineCondition, passportClerkLock);
            SetSyscall(/*me->state*/) = BUSY;
        } else if(GetSyscall(/*me->lineCount*/) > 0) { /* then check regular line */
            Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
            Signal(me->lineCondition, passportClerkLock);
            me->state = BUSY;
        } else {
            
            if(storeJustOpened >= NUM_CLERKS * 4) {
                Print("Passport Clerk %i is going on break\n", 37, myLine * 1000, 0);
                Release(passportClerkLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;
                
                /* Mesa style monitor, wait to be woken up by manager */
                if(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }
                
                Print("Passport Clerk %i is coming off break\n", 39, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(passportClerkLock);
                
                /* Tell the next customer in line that clerk is ready */
                if(me->bribeLineCount > 0) { /* Check bribe line first... */
                    Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
                    Signal(me->bribeLineCondition, passportClerkLock);
                    me->state = BUSY;
                } else if(me->lineCount > 0) { /* then check regular line */
                    Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
                    Signal(me->lineCondition, passportClerkLock);
                    me->state = BUSY;
                }
                else
                    me->state = AVAILABLE;
            }
            else {
                me->state=AVAILABLE;
                storeJustOpened++;
            }
        }
        
        /* entering transaction so switch locks */
        Acquire(me->clerkLock);
        Release(passportClerkLock);
        
        /* wait for customer data */
        Wait(me->clerkCondition, me->clerkLock);
        
        
        Print("Passport Clerk %i has received SSN from Customer %i\n", 53, myLine * 1000 + me->customer->id, 0);
        Print("Passport Clerk %i has determined that Customer %i does have both their application and picture completed\n", 106, myLine * 1000 + me->customer->id, 0);
        for(i = 0; i < Rand()% 80 + 20; i++) {
            Yield();
        }
        me->customer->passportCertified = TRUE;
        Print("Passport Clerk %i has recorded Customer %i passport documentation\n", 67, myLine * 1000 + me->customer->id, 0);
        Signal(me->clerkCondition, me->clerkLock);
        Release(me->clerkLock);
    }
    Exit(0);

}
