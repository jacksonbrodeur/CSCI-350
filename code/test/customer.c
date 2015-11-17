#include "syscall.h"

void pictureTransaction(Clerk * clerk, Customer * customer) {
    
    Acquire(clerk->clerkLock);
    clerk->customer = customer;
    Signal(clerk->clerkCondition, clerk->clerkLock);
    Print("Customer %i has given SSN to PictureClerk %i\n", 46, customer->id * 1000 + clerk->myLine, 0);
    
    while(GetSyscall(customer->pictureTaken) == 0) {
        
        Wait(clerk->clerkCondition, clerk->clerkLock);
        if((Rand() % 10) == 0) {
            Print("Customer %i does not like their picture from PictureClerk %i\n", 62, customer->id * 1000 + clerk->myLine, 0);
        } else {
            Print("Customer %i does like their picture from PictureClerk %i\n", 58, customer->id * 1000 + clerk->myLine, 0);
            customer->pictureTaken = 1;
        }
        Signal(clerk->clerkCondition, clerk->clerkLock);
    }
    
    Release(clerk->clerkLock);
}

void applicationTransaction(Clerk * clerk, Customer * customer) {
    
    Acquire(clerk->clerkLock);
    clerk->customer = customer;
    Print("Customer %i has given SSN to ApplicationClerk %i\n", 67, customer->id * 1000 + clerk->myLine, 0);
    
    Signal(clerk->clerkCondition, clerk->clerkLock);
    
    Wait(clerk->clerkCondition, clerk->clerkLock);
    
    Release(clerk->clerkLock);
    
}

void passportTransaction(Clerk * clerk, Customer * customer) {
    Acquire(clerk->clerkLock);
    clerk->customer = customer;
    
    Print("Customer %i has given SSN to Passport Clerk %i\n", 48, customer->id * 1000 + clerk->myLine, 0);
    
    Signal(clerk->clerkCondition, clerk->clerkLock);
    Wait(clerk->clerkCondition, clerk->clerkLock);
    
    
    Release(clerk->clerkLock);
}

void cashierTransaction(Clerk * clerk, Customer * customer) {
    Acquire(clerk->clerkLock);
    
    clerk->customer = customer;
    
    Signal(clerk->clerkCondition, clerk->clerkLock);
    
    customer->cashierPaid = TRUE;
    Print("Customer %i has given SSN to Cashier %i\n", 41, customer->id * 1000 + clerk->myLine, 0);
    
    Wait(clerk->clerkCondition, clerk->clerkLock);
    
    Acquire(counterLock);
    customersFinished++;
    Release(counterLock);
    
    Print("Customer %i is leaving the passport office\n", 44, customer->id * 1000, 0);
    /*Print("***\n%i/%i\n***\n", 17, customersFinished * 1000 + NUM_CUSTOMERS, 0);*/
    Release(clerk->clerkLock);
}


int getInShortestLine(Customer * customer, Clerk * clerkToVisit, int clerkLock, int clerkType) {
    
    int myLine = -1;
    int shortestLineSize = NUM_CUSTOMERS + NUM_SENATORS;
    int foundLine = FALSE;
    int bribed = FALSE;
    int allOnBreak = TRUE;
    int i;
    
    Acquire(clerkLock);
    
    for(i = 0; i < NUM_CLERKS; i++) {
        if(clerkToVisit[i].state == BUSY || clerkToVisit[i].state == AVAILABLE) {
            allOnBreak = FALSE;
            break;
        }
    }
    
    if(!allOnBreak) {
        
        while (!foundLine) {
            /*
             for(i = 0; i < NUM_CLERKS; i++) {
             if(clerkToVisit[i].lineCount + clerkToVisit[i].bribeLineCount < shortestLineSize && clerkToVisit[i].state != ONBREAK) {
             myLine = i;
             shortestLineSize = clerkToVisit[i].lineCount + clerkToVisit[i].bribeLineCount;
             foundLine = TRUE;
             bribed = FALSE;
             }
             if(clerkType != CASHIER && customer->money >= 600 && clerkToVisit[i].bribeLineCount < shortestLineSize && clerkToVisit[i].state != ONBREAK) {
             myLine = i;
             shortestLineSize = clerkToVisit[i].bribeLineCount;
             foundLine = TRUE;
             bribed = TRUE;
             }
             }*/
            
            if (customer->money >= 600 && clerkToVisit[0].clerkType != CASHIER) {
                
                for(i = 0; i < NUM_CLERKS; i++) {
                    if((clerkToVisit[i].bribeLineCount < shortestLineSize) && (clerkToVisit[i].state != ONBREAK)) {
                        myLine = i;
                        shortestLineSize = clerkToVisit[i].bribeLineCount;
                        foundLine = TRUE;
                        bribed = TRUE;
                    }
                }
            } else {
                for(i = 0; i < NUM_CLERKS; i++) {
                    if((clerkToVisit[i].lineCount + clerkToVisit[i].bribeLineCount < shortestLineSize) && (clerkToVisit[i].state != ONBREAK)) {
                        myLine = i;
                        shortestLineSize = clerkToVisit[i].lineCount + clerkToVisit[i].bribeLineCount;
                        foundLine = TRUE;
                    }
                }
            }
            
            
            
        }
    }
    else { /* all clerks of that type are on break */
        if(customer->money >= 600 && clerkToVisit[0].clerkType != CASHIER) {
            shortestLineSize = clerkToVisit[0].bribeLineCount;
            bribed = TRUE;
        } else {
            shortestLineSize = clerkToVisit[0].bribeLineCount + clerkToVisit[0].lineCount;
        }
        myLine = 0;
        foundLine = TRUE;
    }
    
    if(clerkToVisit[myLine].state == BUSY || clerkToVisit[myLine].state == ONBREAK) {
        if(bribed) {
            if (clerkType == PICTURECLERK)
                Print("Customer %i has gotten in bribe line for PictureClerk %i\n", 58, customer->id * 1000 + myLine, 0);
            else if (clerkType == APPLICATIONCLERK)
                Print("Customer %i has gotten in bribe line for ApplicationClerk %i\n", 62, customer->id * 1000 + myLine, 0);
            else if (clerkType == PASSPORTCLERK)
                Print("Customer %i has gotten in bribe line for PassportClerk %i\n", 59, customer->id * 1000 + myLine, 0);
            else /* clerkType == CASHIER */
                Print("Customer %i has gotten in bribe line for Cashier %i\n", 53, customer->id * 1000 + myLine, 0);
            
            clerkToVisit[myLine].bribeLineCount++;
            Wait(clerkToVisit[myLine].bribeLineCondition, clerkLock);
            clerkToVisit[myLine].bribeLineCount--;
        } else {
            if (clerkType == PICTURECLERK)
                Print("Customer %i has gotten in regular line for PictureClerk %i\n", 60, customer->id * 1000 + myLine, 0);
            else if (clerkType == APPLICATIONCLERK)
                Print("Customer %i has gotten in regular line for ApplicationClerk %i\n", 64, customer->id * 1000 + myLine, 0);
            else if (clerkType == PASSPORTCLERK)
                Print("Customer %i has gotten in regular line for PassportClerk %i\n", 61, customer->id * 1000 + myLine, 0);
            else /* clerkType == CASHIER */
                Print("Customer %i has gotten in regular line for Cashier %i\n", 55, customer->id * 1000 + myLine, 0);
            
            clerkToVisit[myLine].lineCount++;
            Wait(clerkToVisit[myLine].lineCondition, clerkLock);
            clerkToVisit[myLine].lineCount--;
        }
    }
    
    if (bribed)
    {
        customer->money -= 500;
        clerkToVisit[myLine].money += 500;
    }
    
    clerkToVisit[myLine].state = BUSY;
    Release(clerkLock);
    
    return myLine;
}

main()
{
    Customer tempCustomer;
    int id;
    Customer * me;
    int myLine;
    int i;
    
    Acquire(counterLock);
    id = numCustomers;
    numCustomers++;
    me = &customers[id];
    Release(counterLock);
    /*Print("***\nCustomer %i has been created\n***\n", 40, id * 1000, 0);*/
    
    /*Increment the number of senators currently using the office*/
    if(me->isSenator)
    {
        Acquire(senatorLock);
        numSenatorsHere++;
        Release(senatorLock);
    }
    /*Make any customer who enters the office wait if there are any senators currently using the office */
    while((!me->isSenator) && numSenatorsHere > 0) {
        
        Acquire(senatorLock);
        Wait(senatorCondition, senatorLock);
        Release(senatorLock);
    }
    
    /*Randomly decide to go to the picture clerk first or application clerk first*/
    if(Rand() % 2 == 0)
    {
        /*find the shortest line and then perform a transaction with the clerk of that line*/
        myLine = getInShortestLine(me, pictureClerks, pictureClerkLock, PICTURECLERK);
        pictureTransaction(&pictureClerks[myLine], me);
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while(numSenatorsHere>0 && !(me->isSenator))
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, me->id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }
        
        /*go to the application clerk second*/
        myLine = getInShortestLine(me, applicationClerks, applicationClerkLock, APPLICATIONCLERK);
        applicationTransaction(&applicationClerks[myLine], me);
    }
    else
    {
        /*find the shortest line and then perform a transaction with the clerk of that line*/
        myLine = getInShortestLine(me, applicationClerks, applicationClerkLock, APPLICATIONCLERK);
        applicationTransaction(&applicationClerks[myLine], me);
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while(GetSyscall(numSenatorsHere)>0 && !(me->isSenator))
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, me->id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }
        
        /*go to the application clerk second*/
        myLine = getInShortestLine(me, pictureClerks, pictureClerkLock, PICTURECLERK);
        pictureTransaction(&pictureClerks[myLine], me);
    }
    
    /*do not go any further until the customer has gotten his passport certified*/
    while(!me->passportCertified) {
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while(numSenatorsHere>0 && !(me->isSenator))
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, me->id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }
        
        /*punish the customer if they tried to go to the passport clerk before the app and/or picture was filed*/
        if(!me->pictureFiled || !me->applicationFiled) {
            for(i = 0; i < Rand() % 900 + 100; i++) {
                Yield();
            }
        } else { /*if the customer  has gotten both app and picture filed then go to passport clerk*/
            myLine = getInShortestLine(me, passportClerks, passportClerkLock, PASSPORTCLERK);
            passportTransaction(&passportClerks[myLine], me);
        }
    }
    
    /*make sure the customer pays */
    while(!me->cashierPaid) {
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while(numSenatorsHere>0 && !(me->isSenator))
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, me->id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }
        
        /*punish the customer if he tries to pay before his passport is certified */
        if(!me->passportCertified) {
            for(i = 0; i < Rand() % 900 + 100; i++) {
                Yield();
            }
        } else { /*if his passport is certified then let the customer pay*/
            myLine = getInShortestLine(me, cashiers, cashierLock, CASHIER);
            cashierTransaction(&cashiers[myLine], me);
        }
    }
    
    /*1 senator is done using the office so decrement the counter of senators currently in the office and signal all waiting customers if the last senator is leaving the office*/
    if(me->isSenator)
    {
        Acquire(senatorLock);
        numSenatorsHere--;
        if(numSenatorsHere==0) {
            storeJustOpened=0;
            Broadcast(senatorCondition, senatorLock);
        }
        Release(senatorLock);
    }
    Exit(0);
}
