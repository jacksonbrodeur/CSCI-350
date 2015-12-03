#include "syscall.h"
#include "setup.h"

main()
{
    int myLine;
    int i;
    int j = Rand() % 80 + 20;
    int lineCondition;
    int bribeLineCondition;
    int clerkCondition;
    int breakLock;
    int breakCondition;
    int clerkLock;
    int customerID;

    int clerkType;
    int money;

    Acquire(counterLock);
    myLine = Get(numApplicationClerks, 0);
    Set(numApplicationClerks, 0, myLine + 1);
    Release(counterLock);

    lineCondition = Get(appLineCV, myLine);
    bribeLineCondition = Get(appBribeLineCV, myLine);

    clerkCondition = Get(appClerkCV, 0);


    breakLock = Get(appBreakLock, myLine);
    breakCondition = Get(appBreakCV, myLine);

    clerkLock = Get(appClerkLock, myLine);
    
    while(Get(customersFinished, 0) < NUM_CUSTOMERS + NUM_SENATORS) {
        
        Acquire(applicationClerkLock);
        
        /* If there is a customer in line signal him to the counter */
        if(Get(appBribeLineCount, myLine) > 0) {
            Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
            Signal(bribeLineCondition, applicationClerkLock);
            Set(appState,myLine, BUSY);
        } else if(Get(appLineCount, myLine) > 0) {
            Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
            Signal(lineCondition, applicationClerkLock);
            Set(appState, myLine, BUSY);
        } else {
            
            if(storeJustOpened >= NUM_CLERKS * 4) {
                Print("ApplicationClerk %i is going on break\n", 39, myLine * 1000, 0);
                Release(applicationClerkLock);
                Acquire(breakLock);
                Set(appState, myLine, ONBREAK);
                
                if(Get(appState, myLine) == ONBREAK) {
                    Wait(breakCondition, breakLock);
                }
                
                Print("ApplicationClerk %i is coming off break\n", 41, myLine * 1000, 0);
                Release(breakLock);
                Acquire(applicationClerkLock);
                
                if(Get(appBribeLineCount, myLine) > 0) {
                    Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
                    Signal(bribeLineCondition, applicationClerkLock);
                    Set(appState, myLine, BUSY);
                } else if(Get(appLineCount, myLine) > 0) {
                    Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 71, myLine * 1000, 0);
                    Signal(lineCondition, applicationClerkLock);
                    Set(appState, myLine, BUSY);
                }
                else {
                    Set(appState, myLine, AVAILABLE);
                }
                
            } else {
                Set(appState, myLine, AVAILABLE);
                Set(storeJustOpened, 0, Get(storeJustOpened, 0) + 1);
            }
        }
        
        Acquire(clerkLock);
        Release(applicationClerkLock);
        
        Wait(clerkCondition, clerkLock);

        customerID = Get(appCustomer,myLine);
        
        Print("ApplicationClerk %i has received SSN from Customer %i\n", 55, myLine * 1000 + customerID, 0);
        for(i = 0; i < j; i++) {
            Yield();
        }
        Set(applicationFiled, customerID, TRUE);
        Print("ApplicationClerk %i has recorded a completed application for Customer %i\n", 74, myLine * 1000 + customerID, 0);
        
        Signal(clerkCondition, clerkLock);
        
        Release(clerkLock);
    }
    Exit(0);
}
