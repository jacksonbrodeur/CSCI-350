#include "syscall.h"

main()
{
    Clerk tempClerk;
    Clerk * me;
    int myLine;
    
    Acquire(counterLock);
    myLine = numCashiers;
    numCashiers++;
    me = &cashiers[myLine];
    Release(counterLock);
    
    /* On duty while there are still customers who haven't completed process */
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(cashierLock);
        
        if (me->lineCount > 0) {
            Print("Cashier %i has signalled a customer to come to their counter\n", 62, myLine * 1000, 0);
            Signal(me->lineCondition, cashierLock);
            me->state = BUSY;
        } else {
            if(storeJustOpened>=NUM_CLERKS*4) {
                Print("Cashier %i is going on break\n", 30, myLine * 1000, 0);
                Release(cashierLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;
                
                /* Mesa style monitor, wait to be woken up by manager */
                if(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }
                
                Print("Cashier %i is coming off break\n", 32, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(cashierLock);
                
                if (me->lineCount > 0) {
                    Print("Cashier %i has signalled a customer to come to their counter\n", 62, myLine * 1000, 0);
                    Signal(me->lineCondition, cashierLock);
                    me->state = BUSY;
                }
                else
                    me->state = AVAILABLE;
            }
            else {
                me->state = AVAILABLE;
                storeJustOpened++;
            }
        }
        
        /* entering transaction so switch locks */
        Acquire(me->clerkLock);
        Release(cashierLock);
        
        /* wait for customer to pay */
        Wait(me->clerkCondition, me->clerkLock);
        
        Print("Cashier %i has received SSN from Customer %i\n", 46, myLine * 1000 + me->customer->id, 0);
        Print("Cashier %i has verified that Customer %i has been certified by a PassportClerk\n", 80, myLine * 1000 + me->customer->id, 0);
        /* taking payment */
        me->customer->money -= 100;
        me->money += 100;
        Print("Cashier %i has received the $100 from Customer %i after certification\n", 71, myLine * 1000 + me->customer->id, 0);
        
        me->customer->passportGiven = TRUE;
        Signal(me->clerkCondition, me->clerkLock);
        Print("Cashier %i has provided Customer %i their completed passport\n", 62, myLine * 1000 + me->customer->id, 0);
        Print("Cashier %i has recorded that Customer %i has been given their completed passport\n", 82, myLine * 1000 + me->customer->id, 0);
        
        Release(me->clerkLock);
    }   
    Exit(0);

}
