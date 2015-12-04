#include "syscall.h"
#include "setup.h"

void pictureTransaction(int clerkID, int customerID) {
    int clerkLock = Get(picClerkLock, clerkID);
    int clerkCV = Get(picClerkCV, clerkID);
    Acquire(clerkLock);
    Set(picCustomer, clerkID, customerID);
    Signal(clerkCV, clerkLock);
    Print("Customer %i has given SSN to PictureClerk %i\n", 46, customerID * 1000 + clerkID, 0);
    
    while(Get(pictureTaken, customerID) == 0) {
        Wait(clerkCV, clerkLock);
        if((Rand() % 10) == 0) {
            Print("Customer %i does not like their picture from PictureClerk %i\n", 62, customerID * 1000 + clerkID, 0);
        } else {
            Print("Customer %i does like their picture from PictureClerk %i\n", 58, customerID * 1000 + clerkID, 0);
            Set(pictureTaken, customerID, 1);
        }
        Signal(clerkCV, clerkLock);
    }
    
    Release(clerkLock);
}

void applicationTransaction(int clerkID, int customerID) {
    int clerkLock = Get(appClerkLock, clerkID);
    int clerkCV = Get(appClerkCV, clerkID);
    Acquire(clerkLock);
    Set(appCustomer, clerkID, customerID);
    Print("Customer %i has given SSN to ApplicationClerk %i\n", 67, customerID * 1000 + clerkID, 0);
    
    Signal(clerkCV, clerkLock);
    
    Wait(clerkCV, clerkLock);
    
    Release(clerkLock);
    
}

void passportTransaction(int clerkID, int customerID) {
    int clerkLock = Get(ppClerkLock, clerkID);
    int clerkCV = Get(ppClerkCV, clerkID);
    Acquire(clerkLock);
    Set(ppCustomer, clerkID, customerID);    
    Print("Customer %i has given SSN to Passport Clerk %i\n", 48, customerID * 1000 + clerkID, 0);
    
    Signal(clerkCV, clerkLock);
    Wait(clerkCV, clerkLock);
    
    
    Release(clerkLock);
}

void cashierTransaction(int clerkID, int customerID) {
    int clerkLock = Get(cashClerkLock, clerkID);
    int clerkCV = Get(cashClerkCV, clerkID);

    Acquire(clerkLock);
    
    Set(cashCustomer, clerkID, customerID);    
    Signal(clerkCV, clerkLock);
    
    Set(cashierPaid, customerID, TRUE);
    Print("Customer %i has given SSN to Cashier %i\n", 41, customerID * 1000 + clerkID, 0);
    
    Wait(clerkCV, clerkLock);
    
    Acquire(counterLock);
    /*increment customersFinished */
    Set(customersFinished, 0, Get(customersFinished, 0) + 1);
    Release(counterLock);
    
    Print("Customer %i is leaving the passport office\n", 44, customerID * 1000, 0);
    Release(clerkLock);
}


int getInShortestLine(int customerID, int clerkLock, int clerkType) {
    
    int myLine = -1;
    int shortestLineSize = Get(NUM_CUSTOMERS, 0) + Get(NUM_SENATORS, 0);
    int foundLine = FALSE;
    int bribed = FALSE;
    int allOnBreak = TRUE;
    int i;
    int state;
    int numClerks = Get(NUM_CLERKS, 0);
    int bribeLineSize;
    int lineSize;
    
    Acquire(clerkLock);
    
    for(i = 0; i < Get(NUM_CLERKS, 0); i++) {
        if(clerkType == PICTURECLERK) {
            state = Get(picState, i);
        } else if (clerkType == APPLICATIONCLERK) {
            state = Get(appState, i);
        } else if (clerkType == PASSPORTCLERK) {
            state = Get(ppState, i);
        } else if (clerkType == CASHIER) {
            state = Get(cashState, i);
        }

        if(state == BUSY || state == AVAILABLE) {
            allOnBreak = FALSE;
            break;
        }
    }
    
    if(!allOnBreak) {
        
        while (!foundLine) {
            if (Get(custMoney, customerID) >= 600) {
                
                for(i = 0; i < numClerks; i++) {
                    if (clerkType == PICTURECLERK) {
                        bribeLineSize = Get(picBribeLineCount, i);
                        state = Get(picState, i);
                    } else if (clerkType == APPLICATIONCLERK) {
                        bribeLineSize = Get(appBribeLineCount, i);
                        state = Get(appState, i);
                    } else if (clerkType == PASSPORTCLERK) {
                        bribeLineSize = Get(ppBribeLineCount, i);
                        state = Get(ppState, i);
                    } else if (clerkType == CASHIER) {
                        bribeLineSize = Get(cashBribeLineCount, i);
                        state = Get(cashState, i);
                    }
                    if((bribeLineSize < shortestLineSize) && (state != ONBREAK)) {
                        myLine = i;
                        shortestLineSize = bribeLineSize;
                        foundLine = TRUE;
                        bribed = TRUE;
                    }
                }
            } else {
                for(i = 0; i < numClerks; i++) {
                    if (clerkType == PICTURECLERK){
                        bribeLineSize = Get(picBribeLineCount, i);
                        lineSize = Get(picLineCount, i);
                        state = Get(picState, i);
                    } else if (clerkType == APPLICATIONCLERK) {
                        bribeLineSize = Get(appBribeLineCount, i);
                        lineSize = Get(appLineCount, i);
                        state = Get(appState, i);
                    } else if (clerkType == PASSPORTCLERK) {
                        bribeLineSize = Get(ppBribeLineCount, i);
                        lineSize = Get(ppLineCount, i);
                        state = Get(ppState, i);
                    } else if (clerkType == CASHIER) {
                        bribeLineSize = Get(cashBribeLineCount, i);
                        lineSize = Get(cashLineCount, i);
                        state = Get(cashState, i);
                    }
                    if((lineSize + bribeLineSize < shortestLineSize) && (state != ONBREAK)) {
                        myLine = i;
                        shortestLineSize = lineSize + bribeLineSize;
                        foundLine = TRUE;
                    }
                }
            }
            
            
            
        }
    }
    else { /* all clerks of that type are on break */
        if(Get(custMoney, customerID) >= 600) {
            if (clerkType == PICTURECLERK) {
                bribeLineSize = Get(picBribeLineCount, 0);
            } else if (clerkType == APPLICATIONCLERK) {
                bribeLineSize = Get(appBribeLineCount, 0);
            } else if (clerkType == PASSPORTCLERK) {
                bribeLineSize = Get(ppBribeLineCount, 0);
            } else if (clerkType == CASHIER) {
                bribeLineSize = Get(cashBribeLineCount, 0);
            }
            shortestLineSize = bribeLineSize;
            bribed = TRUE;
        } else {
            if (clerkType == PICTURECLERK) {
                bribeLineSize = Get(picBribeLineCount, 0);
                lineSize = Get(picLineCount, 0);
            } else if (clerkType == APPLICATIONCLERK) {
                bribeLineSize = Get(appBribeLineCount, 0);
                lineSize = Get(appLineCount, 0);
            } else if (clerkType == PASSPORTCLERK) {
                bribeLineSize = Get(ppBribeLineCount, 0);
                lineSize = Get(ppLineCount, 0);
            } else if (clerkType == CASHIER) {
                bribeLineSize = Get(cashBribeLineCount, 0);
                lineSize = Get(cashLineCount, 0);
            }
            shortestLineSize = bribeLineSize + lineSize;
        }
        myLine = 0;
        foundLine = TRUE;
    }

    if (clerkType == PICTURECLERK) {
        state = Get(picState, myLine);
    } else if (clerkType == APPLICATIONCLERK) {
        state = Get(appState, myLine);
    } else if (clerkType == PASSPORTCLERK) {
        state = Get(ppState, myLine);
    } else if (clerkType == CASHIER) {
        state = Get(cashState, myLine);
    }
    
    if(state == BUSY || state == ONBREAK) {
        if(bribed) {
            if (clerkType == PICTURECLERK) {
                Print("Customer %i has gotten in bribe line for PictureClerk %i\n", 58, customerID * 1000 + myLine, 0);
                Set(picBribeLineCount, myLine, Get(picBribeLineCount, myLine) + 1);
                Wait(Get(picBribeLineCount, myLine), pictureClerkLock);
                Set(picBribeLineCount, myLine, Get(picBribeLineCount, myLine) - 1);
            } else if (clerkType == APPLICATIONCLERK) {
                Print("Customer %i has gotten in bribe line for ApplicationClerk %i\n", 62, customerID * 1000 + myLine, 0);
                Set(appBribeLineCount, myLine, Get(appBribeLineCount, myLine) + 1);
                Wait(Get(appBribeLineCV, myLine), applicationClerkLock);
                Set(appBribeLineCount, myLine, Get(appBribeLineCount, myLine) - 1);
            } else if (clerkType == PASSPORTCLERK) {
                Print("Customer %i has gotten in bribe line for PassportClerk %i\n", 59, customerID * 1000 + myLine, 0);
                Set(ppBribeLineCount, myLine, Get(ppBribeLineCount, myLine) + 1);
                Wait(Get(ppBribeLineCV, myLine), passportClerkLock);
                Set(ppBribeLineCount, myLine, Get(ppBribeLineCount, myLine) - 1);
            } else { /* clerkType == CASHIER */
                Print("Customer %i has gotten in bribe line for Cashier %i\n", 53, customerID * 1000 + myLine, 0);
                Set(cashBribeLineCount, myLine, Get(cashBribeLineCount, myLine) + 1);
                Wait(Get(cashBribeLineCV, myLine), cashierLock);
                Set(cashBribeLineCount, myLine, Get(cashBribeLineCount, myLine) - 1);
            }
        } else {
            if (clerkType == PICTURECLERK) { 
                Print("Customer %i has gotten in regular line for PictureClerk %i\n", 60, customerID * 1000 + myLine, 0);
                Set(picLineCount, myLine, Get(picLineCount, myLine) + 1);
                Wait(Get(picLineCV, myLine), pictureClerkLock);
                Set(picLineCount, myLine, Get(picLineCount, myLine) - 1);
            } else if (clerkType == APPLICATIONCLERK) {
                Print("Customer %i has gotten in regular line for ApplicationClerk %i\n", 64, customerID * 1000 + myLine, 0);
                Set(appLineCount, myLine, Get(appLineCount, myLine) + 1);
                Wait(Get(appLineCV, myLine), applicationClerkLock);
                Set(appLineCount, myLine, Get(appLineCount, myLine) - 1);
            } else if (clerkType == PASSPORTCLERK) {
                Print("Customer %i has gotten in regular line for PassportClerk %i\n", 61, customerID * 1000 + myLine, 0);
                Set(ppLineCount, myLine, Get(ppLineCount, myLine) + 1);
                Wait(Get(ppLineCV, myLine), passportClerkLock);
                Set(ppLineCount, myLine, Get(ppLineCount, myLine) - 1);
            } else { /* clerkType == CASHIER */
                Print("Customer %i has gotten in regular line for Cashier %i\n", 55, customerID * 1000 + myLine, 0);
                Set(cashLineCount, myLine, Get(cashLineCount, myLine) + 1);
                Wait(Get(cashLineCV, myLine), cashierLock);
                Set(cashLineCV, myLine, Get(cashLineCV, myLine) - 1);
            }
        }
    }
    
    if (bribed)
    {
        /*Remove 500 from customer money*/
        Set(custMoney, customerID, Get(custMoney, customerID) - 500);
        /*Add 500 to appropriate clerk's money*/
        if (clerkType == PICTURECLERK) {
            Set(picMoney, myLine, Get(picMoney, myLine) + 500);
        } else if (clerkType == APPLICATIONCLERK) {
            Set(appMoney, myLine, Get(appMoney, myLine) + 500);
        } else if (clerkType == PASSPORTCLERK) {
            Set(ppMoney, myLine, Get(ppMoney, myLine) + 500);
        } else if (clerkType == CASHIER) {
            Set(cashMoney, myLine, Get(cashMoney, myLine) + 500);
        }
    }
    
    if (clerkType == PICTURECLERK) {
        Set(picState, myLine, BUSY);
    } else if (clerkType == APPLICATIONCLERK) {
        Set(appState, myLine, BUSY);
    } else if (clerkType == PASSPORTCLERK) {
        Set(ppState, myLine, BUSY);
    } else if (clerkType == CASHIER) {
        Set(cashState, myLine, BUSY);
    }
    Release(clerkLock);
    
    return myLine;
}

main()
{
    int id;
    int myLine;
    int i;

    setup();
    
    Acquire(counterLock);
    id = Get(numCustomers, 0);
    Set(numCustomers, 0, id + 1);
    Release(counterLock);
    /*Print("***\nCustomer %i has been created\n***\n", 40, id * 1000, 0);*/
    
    /*Increment the number of senators currently using the office*/
    if(Get(isSenator, id) != 0)
    {
        Acquire(senatorLock);
        Set(numSenatorsHere, 0, Get(numSenatorsHere, 0) + 1);
        Release(senatorLock);
    }
    /*Make any customer who enters the office wait if there are any senators currently using the office */
    while((Get(isSenator, id) == 0) && Get(numSenatorsHere, 0) > 0) {
        
        Acquire(senatorLock);
        Wait(senatorCondition, senatorLock);
        Release(senatorLock);
    }
    
    /*Randomly decide to go to the picture clerk first or application clerk first*/
    if(Rand() % 2 == 0)
    {
        /*find the shortest line and then perform a transaction with the clerk of that line*/
        myLine = getInShortestLine(id, pictureClerkLock, PICTURECLERK);
        pictureTransaction(myLine, id);
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while((Get(isSenator, id) == 0) && Get(numSenatorsHere, 0) > 0)
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }
        
        /*go to the application clerk second*/
        myLine = getInShortestLine(id, applicationClerkLock, APPLICATIONCLERK);
        applicationTransaction(myLine, id);
    }
    else
    {
        /*find the shortest line and then perform a transaction with the clerk of that line*/
        myLine = getInShortestLine(id, applicationClerkLock, APPLICATIONCLERK);
        applicationTransaction(myLine, id);
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while((Get(isSenator, id) == 0) && Get(numSenatorsHere, 0) > 0)
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }

        
        /*go to the application clerk second*/
        myLine = getInShortestLine(id, pictureClerkLock, PICTURECLERK);
        pictureTransaction(myLine, id);
    }
    
    /*do not go any further until the customer has gotten his passport certified*/
    while(Get(passportCertified, id) == FALSE) {
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while((Get(isSenator, id) == 0) && Get(numSenatorsHere, 0) > 0)
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }
        
        /*punish the customer if they tried to go to the passport clerk before the app and/or picture was filed*/
        if(Get(pictureFiled, id) == FALSE || Get(applicationFiled, id) == FALSE) {
            for(i = 0; i < Rand() % 900 + 100; i++) {
                Yield();
            }
        } else { /*if the customer  has gotten both app and picture filed then go to passport clerk*/
            myLine = getInShortestLine(id, passportClerkLock, PASSPORTCLERK);
            passportTransaction(myLine, id);
        }
    }
    
    /*make sure the customer pays */
    while(Get(cashierPaid, id) == FALSE) {
        
        /*if there is a senator in the office, make the customer wait after he is finished with the clerk he is currently using*/
        while((Get(isSenator, id) == 0) && Get(numSenatorsHere, 0) > 0)
        {
            Print("Customer %i is going outside the Passport Office because there is a Senator present\n", 85, id * 1000, 0);
            Acquire(senatorLock);
            Wait(senatorCondition, senatorLock);
            Release(senatorLock);
        }
        
        /*punish the customer if he tries to pay before his passport is certified */
        if(Get(passportCertified, id) == FALSE) {
            for(i = 0; i < Rand() % 900 + 100; i++) {
                Yield();
            }
        } else { /*if his passport is certified then let the customer pay*/
            myLine = getInShortestLine(id, cashierLock, CASHIER);
            cashierTransaction(myLine, id);
        }
    }
    
    /*1 senator is done using the office so decrement the counter of senators currently in the office and signal all waiting customers if the last senator is leaving the office*/
    if(Get(isSenator, id))
    {
        Acquire(senatorLock);
        Set(numSenatorsHere, 0, Get(numSenatorsHere, 0) - 1);
        if(Get(numSenatorsHere, 0) == 0) {
            Set(storeJustOpened, 0, 0);
            Broadcast(senatorCondition, senatorLock);
        }
        Release(senatorLock);
    }
    Exit(0);
}
