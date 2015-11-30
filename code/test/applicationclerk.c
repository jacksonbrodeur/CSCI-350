#include "syscall.h"
#include "setup.h"

#define NUM_CUSTOMERS 20
#define NUM_SENATORS 10

main()
{
    int myLine;
    int i;
    int j = Rand() % 80 + 20;
    int counterLock = GetSyscall(/*counterLock index */);

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
    myLine = CreateSyscall(numApplicationClerks);
    SetSyscall(numApplicationClerks++);
    me = &applicationClerks[myLine];
    Release(counterLock);
    
    while(GetSyscall(customersFinished) < NUM_CUSTOMERS + NUM_SENATORS) {
        
        Acquire(applicationClerkLock);
        
        /* If there is a customer in line signal him to the counter */
        if(GetSyscall(appBribeLineCount) > 0) {
            Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
            Signal(me->bribeLineCondition, applicationClerkLock);
            SetSyscall(appState) = BUSY;
        } else if(GetSyscall(appLineCount) > 0) {
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
