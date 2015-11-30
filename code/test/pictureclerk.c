#include "syscall.h"
#include "setup.h"

main()
{
    int myLine;
    int firstTime = 1;
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
    myLine = CreateSyscall(numPictureClerks);
    SetSyscall(0);
    me = &pictureClerks[myLine];
    Release(counterLock);
    
    while(GetSyscall(customersFinished) < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(pictureClerkLock);
        
        /* If there is a customer in line signal him to the counter */
        if(GetSyscall(picBribeLineCount, myLine) > 0) {
            Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->bribeLineCondition, pictureClerkLock);
            SetSyscall(picState) = BUSY;
        } else if(GetSyscall(picLineCount, myLine) > 0) {
            Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->lineCondition, pictureClerkLock);
            SetSyscall(picState) = BUSY;
        } else {
            
            if(storeJustOpened >= NUM_CLERKS * 4) {
                Print("PictureClerk %i is going on break\n", 35, myLine * 1000, 0);
                Release(pictureClerkLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;
                
                if(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }
                
                Print("PictureClerk %i is coming off break\n", 37, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(pictureClerkLock);
                
                if(me->bribeLineCount > 0) {
                    Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
                    Signal(me->bribeLineCondition, pictureClerkLock);
                    me->state = BUSY;
                } else if(me->lineCount > 0) {
                    Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
                    Signal(me->lineCondition, pictureClerkLock);
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
        Release(pictureClerkLock);
        
        Wait(me->clerkCondition, me->clerkLock);
        Print("PictureClerk %i has received SSN from Customer %i\n", 51, myLine * 1000 + me->customer->id, 0);
        
        while(me->customer->pictureTaken == 0) {
            
            if(firstTime != 1) {
                Print("PictureClerk %i has been told that Customer %i does not like their picture\n", 76, myLine * 1000 + me->customer->id, 0);
            }
            Print("PictureClerk %i has taken a picture of Customer %i\n", 52, myLine * 1000 + me->customer->id, 0);
            Signal(me->clerkCondition, me->clerkLock);
            Wait(me->clerkCondition, me->clerkLock);
            firstTime = 0;
        }
        
        Print("PictureClerk %i has been told that Customer %i does like their picture\n", 72, myLine * 1000 + me->customer->id, 0);
        
        for(i = 0; i < j; i++) {
            Yield();
        }
        
        me->customer->pictureFiled = 1;
        Signal(me->clerkCondition, me->clerkLock);
        Release(me->clerkLock);
    }
    Exit(0);
    

}
