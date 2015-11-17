#include "syscall.h"

main()
{
    Clerk tempClerk;
    int myLine;
    int i;
    int j = Rand() % 80 + 20;
    Clerk * me;
    
    Acquire(counterLock);
    myLine = numApplicationClerks;
    numApplicationClerks++;
    me = &applicationClerks[myLine];
    Release(counterLock);
    
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        
        Acquire(applicationClerkLock);
        
        /* If there is a customer in line signal him to the counter */
        if(me->bribeLineCount > 0) {
            Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
            Signal(me->bribeLineCondition, applicationClerkLock);
            me->state = BUSY;
        } else if(me->lineCount > 0) {
            Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
            Signal(me->lineCondition, applicationClerkLock);
            me->state = BUSY;
        } else {
            
            if(storeJustOpened >= NUM_CLERKS * 4) {
                Print("ApplicationClerk %i is going on break\n", 39, myLine * 1000, 0);
                Release(applicationClerkLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;
                
                if(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }
                
                Print("ApplicationClerk %i is coming off break\n", 41, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(applicationClerkLock);
                
                if(me->bribeLineCount > 0) {
                    Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
                    Signal(me->bribeLineCondition, applicationClerkLock);
                    me->state = BUSY;
                } else if(me->lineCount > 0) {
                    Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
                    Signal(me->lineCondition, applicationClerkLock);
                    me->state = BUSY;
                }
                else
                    me->state = AVAILABLE;
                
            } else {
                me->state = AVAILABLE;
                storeJustOpened++;
            }
        }
        
        Acquire(me->clerkLock);
        Release(applicationClerkLock);
        
        Wait(me->clerkCondition, me->clerkLock);
        
        Print("ApplicationClerk %i has received SSN from Customer %i\n", 55, myLine * 1000 + me->customer->id, 0);
        for(i = 0; i < j; i++) {
            Yield();
        }
        me->customer->applicationFiled = TRUE;
        Print("ApplicationClerk %i has recorded a completed application for Customer %i\n", 74, myLine * 1000 + me->customer->id, 0);
        
        Signal(me->clerkCondition, me->clerkLock);
        
        Release(me->clerkLock);
    }
    Exit(0);
}
