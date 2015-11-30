#include "syscall.h"
#include "setup.h"

main()
{
    int myLine;
    int firstTime = 1;
    int i;
    int j = Rand() % 80 + 20;

    int clerkType;
    int money;

    Acquire(counterLock);
   myLine = Get(numPictureClerks, 0);
   Set(numPictureClerks, 0, myLine + 1);
    me = &pictureClerks[myLine];
    Release(counterLock);

    int lineCondition = Get(picLineCV, myLine);
    int bribeLineCondition = Get(picBribeLineCV, myLine);

    int clerkCondition = Get(picClerkCV, 0);


    int breakLock = Get(picBreakLock, myLine);
    int breakCondition = Get(picBreakCV, myLine);

    int clerkLock = Get(picClerkLock, myLine);
    
    while(Get(customersFinished, 0) < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(pictureClerkLock);
        
        /* If there is a customer in line signal him to the counter */
        if(Get(picBribeLineCount, myLine) > 0) {
            Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->bribeLineCondition, pictureClerkLock);
            Set(picState, myLine, BUSY);
        } else if(Get(picLineCount, myLine) > 0) {
            Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->lineCondition, pictureClerkLock);
            Set(picState, myLine, BUSY);
        } else {
            
            if(Get(storeJustOpened,0) >= NUM_CLERKS * 4) {
                Print("PictureClerk %i is going on break\n", 35, myLine * 1000, 0);
                Release(pictureClerkLock);
                Acquire(breakLock);
                Set(picState, myLine, ONBREAK);
                
                if(Get(picState, myLine, ONBREAK)) {
                    Wait(breakCondition,breakLock);
                }
                
                Print("PictureClerk %i is coming off break\n", 37, myLine * 1000, 0);
                Release(breakLock);
                Acquire(pictureClerkLock);
                
                if(Get(picBribeLineCount, myLine) > 0) {
                    Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
                    Signal(bribeLineCondition, pictureClerkLock);
                    Set(picState, myLine, BUSY);
                } else if(Get(piclineCount, myLine) > 0) {
                    Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
                    Signal(lineCondition, pictureClerkLock);
                    Set(picState, myLine, BUSY);
                }
                else
                    Set(picState, myLine, AVAILABLE);
                
            } else {
                Set(picState, myLine, AVAILABLE);
                Set(storeJustOpened, 0, Get(storeJustOpened,0));
            }
        }
        
        Acquire(clerkLock);
        Release(pictureClerkLock);
        
        Wait(clerkCondition, clerkLock);
        int customerID = Get(picCustomer, myLine);
        Print("PictureClerk %i has received SSN from Customer %i\n", 51, myLine * 1000 + customerID, 0);
        
        while(Get(picCustomer)->pictureTaken == 0) {
            
            if(firstTime != 1) {
                Print("PictureClerk %i has been told that Customer %i does not like their picture\n", 76, myLine * 1000 + me->customer->id, 0);
            }
            Print("PictureClerk %i has taken a picture of Customer %i\n", 52, myLine * 1000 + customerID, 0);
            Signal(clerkCondition, clerkLock);
            Wait(clerkCondition, clerkLock);
            firstTime = 0;
        }
        
        Print("PictureClerk %i has been told that Customer %i does like their picture\n", 72, myLine * 1000 + customerID, 0);
        
        for(i = 0; i < j; i++) {
            Yield();
        }
        
        Get(pictureFiled, customerID, true);
        Signal(clerkCondition, clerkLock);
        Release(clerkLock);
    }
    Exit(0);
    

}
