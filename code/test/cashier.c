#include "syscall.h"
#include "setup.h"

main()
{
    int myLine;
    int lineCondition;
    int bribeLineCondition;
    int clerkCondition;
    int breakLock;
    int breakCondition;
    int clerkLock;
    int customerID;
    
    setup();


    Acquire(counterLock);
    myLine = Get(numCashiers,0);
    Set(numCashiers,0,myLine+1);
    Release(counterLock);


    
    lineCondition = Get(cashLineCV,myLine);
    bribeLineCondition = Get(cashBribeLineCV,myLine);
    clerkCondition = Get(cashClerkCV,myLine);
    
    breakLock = Get(cashBreakLock,myLine);
    breakCondition = Get(cashBreakCV,myLine);
    
    clerkLock = Get(cashClerkLock,myLine);
    
    /* On duty while there are still customers who haven't completed process */
    while(Get(customersFinished,0) < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(cashierLock);
        
        if (Get(cashLineCount,myLine) > 0) {
            Print("Cashier %i has signalled a customer to come to their counter\n", 62, myLine * 1000, 0);
            Signal(lineCondition, cashierLock);
            Set(cashState,myLine,BUSY);
        } else {
            if(Get(storeJustOpened,0)>=NUM_CLERKS*4) {
                Print("Cashier %i is going on break\n", 30, myLine * 1000, 0);
                Release(cashierLock);
                Acquire(breakLock);
                Set(cashState,myLine,ONBREAK);
                
                /* Mesa style monitor, wait to be woken up by manager */
                if(Get(cashState,myLine) == ONBREAK) {
                    Wait(breakCondition, breakLock);
                }
                
                Print("Cashier %i is coming off break\n", 32, myLine * 1000, 0);
                Release(breakLock);
                Acquire(cashierLock);
                
                if (Get(cashLineCount,myLine) > 0) {
                    Print("Cashier %i has signalled a customer to come to their counter\n", 62, myLine * 1000, 0);
                    Signal(lineCondition, cashierLock);
                    Set(cashState,myLine,BUSY);
                }
                else
                    Set(cashState,myLine,AVAILABLE);
            }
            else {
                Set(cashState,myLine,AVAILABLE);
                Set(storeJustOpened,0,Get(storeJustOpened,0)+1);
            }
        }
        
        /* entering transaction so switch locks */
        Acquire(clerkLock);
        Release(cashierLock);
        
        /* wait for customer to pay */
        Wait(clerkCondition, clerkLock);
        
        customerID = Get(cashCustomer,myLine);
        
        Print("Cashier %i has received SSN from Customer %i\n", 46, myLine * 1000 + customerID, 0);
        Print("Cashier %i has verified that Customer %i has been certified by a PassportClerk\n", 80, myLine * 1000 + customerID, 0);
        /* taking payment */
        Set(custMoney,customerID, Get(custMoney,customerID)-100);
        Set(cashMoney,myLine,Get(cashMoney,myLine)+100);
        Print("Cashier %i has received the $100 from Customer %i after certification\n", 71, myLine * 1000 + customerID, 0);
        
        Set(passportGiven,customerID, TRUE);
        Signal(clerkCondition, clerkLock);
        Print("Cashier %i has provided Customer %i their completed passport\n", 62, myLine * 1000 + customerID, 0);
        Print("Cashier %i has recorded that Customer %i has been given their completed passport\n", 82, myLine * 1000 + customerID, 0);
        
        Release(clerkLock);
    }   
    Exit(0);

}
