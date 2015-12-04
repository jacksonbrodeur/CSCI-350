#include "syscall.h"
#include "setup.h"

main()
{
    int myLine;
    int i;
    int lineCondition;
    int bribeLineCondition;
    int clerkCondition;
    int breakLock;
    int breakCondition;
    int clerkLock;
    int customerID;
    
    int clerkType;
    int money;

    setup();
    
    Acquire(counterLock);
    myLine = Get(numPassportClerks, 0);
    Set(numPassportClerks, 0, myLine + 1);
    Release(counterLock);
    
    lineCondition = Get(ppLineCV, myLine);
    bribeLineCondition = Get(ppBribeLineCV, myLine);
    
    clerkCondition = Get(ppClerkCV, 0);
    
    breakLock = Get(ppBreakLock, myLine);
    breakCondition = Get(ppBreakCV, myLine);
    
    clerkLock = Get(ppClerkLock, myLine);

    
    /* On duty while there are still customers who haven't completed process */
    while(Get(customersFinished, 0) < NUM_SENATORS + NUM_CUSTOMERS) {
        
        Acquire(passportClerkLock);
        
        /* If there is a customer in line signal him to the counter */
        if(Get(ppBribeLineCount,myLine) > 0) { /* Check bribe line first... */
            Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
            Signal(bribeLineCondition, passportClerkLock);
            Set(ppState,myLine,BUSY);
        } else if(Get(ppLineCount,myLine) > 0) { /* then check regular line */
            Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
            Signal(lineCondition, passportClerkLock);
            Set(ppState,myLine,BUSY);
        } else {
            
            if(Get(storeJustOpened,0) >= NUM_CLERKS * 4) {
                Print("Passport Clerk %i is going on break\n", 37, myLine * 1000, 0);
                Release(passportClerkLock);
                Acquire(breakLock);
                Set(ppState,myLine,ONBREAK);
                
                /* Mesa style monitor, wait to be woken up by manager */
                if(Get(ppState,myLine) == ONBREAK) {
                    Wait(breakCondition, breakLock);
                }
                
                Print("Passport Clerk %i is coming off break\n", 39, myLine * 1000, 0);
                Release(breakLock);
                Acquire(passportClerkLock);
                
                /* Tell the next customer in line that clerk is ready */
                if(Get(ppBribeLineCount,myLine) > 0) { /* Check bribe line first... */
                    Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
                    Signal(bribeLineCondition, passportClerkLock);
                    Set(ppState,myLine,BUSY);
                } else if(Get(ppLineCount,myLine) > 0) { /* then check regular line */
                    Print("Passport Clerk %i has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
                    Signal(lineCondition, passportClerkLock);
                    Set(ppState,myLine,BUSY);
                }
                else
                    Set(ppState,myLine,AVAILABLE);
            }
            else {
                Set(ppState,myLine,AVAILABLE);
                Set(storeJustOpened,0,Get(storeJustOpened,0) + 1);
            }
        }
        
        /* entering transaction so switch locks */
        Acquire(clerkLock); /*TODO: These acquire/release statements may need to be reversed*/
        Release(passportClerkLock);
        
        /* wait for customer data */
        Wait(clerkCondition, clerkLock);
        
        
        customerID = Get(ppCustomer,myLine);
        
        Print("Passport Clerk %i has received SSN from Customer %i\n", 53, myLine * 1000 + customerID, 0);
        Print("Passport Clerk %i has determined that Customer %i does have both their application and picture completed\n", 106, myLine * 1000 + customerID, 0);
        
        
        for(i = 0; i < Rand()% 80 + 20; i++) {
            Yield();
        }
        Set(passportCertified,customerID,TRUE);
        Print("Passport Clerk %i has recorded Customer %i passport documentation\n", 67, myLine * 1000 + customerID, 0);
        Signal(clerkCondition, clerkLock);
        Release(clerkLock);
    }
    Exit(0);

}
