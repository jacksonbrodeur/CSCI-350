#include "syscall.h"
#include "setup.h"

int main()
{
    int signalPictureClerk;
    int signalAppClerk;
    int signalPassportClerk;
    int signalCashier;
    
    int pictureClerksAllOnBreak;
    int applicationClerksAllOnBreak;
    int passportClerksAllOnBreak;
    int cashiersAllOnBreak;
    
    int pictureRevenue;
    int applicationRevenue;
    int passportRevenue;
    int cashierRevenue;

    int pictureState;
    int applicationState;
    int passportState;
    int cashierState;

    
    int i;

    setup();
    Print("%i\n", 4,Get(customersFinished, 0) * 1000, 0);
    while(Get(customersFinished, 0) < NUM_CUSTOMERS + NUM_SENATORS) {
        
        signalPictureClerk = FALSE;
        signalAppClerk = FALSE;
        signalPassportClerk = FALSE;
        signalCashier = FALSE;
        
        pictureClerksAllOnBreak = TRUE;
        applicationClerksAllOnBreak = TRUE;
        passportClerksAllOnBreak = TRUE;
        cashiersAllOnBreak = TRUE;

        /*see if 1) there are any clerks with 3 or more customers waiting on them and 2) if all clerks of a certain type are on break*/
        for(i = 0; i < NUM_CLERKS; i++)
        {
            if(Get(picLineCount, i) + Get(picBribeLineCount, i) >= 3)
                signalPictureClerk = TRUE;
            if(Get(appLineCount, i) + Get(appLineCount, i) >= 3)
                signalAppClerk = TRUE;
            if(Get(ppLineCount, i) + Get(ppBribeLineCount, i) >= 3)
                signalPassportClerk = TRUE;
            if(Get(cashLineCount, i) >= 3)
                signalCashier = TRUE;
            pictureState = Get(picState, i);
            if(pictureState == BUSY || pictureState == AVAILABLE)
                pictureClerksAllOnBreak = FALSE;
            applicationState = Get(appState, i);
            if(applicationState == BUSY || applicationState == AVAILABLE)
                applicationClerksAllOnBreak = FALSE;
            passportState = Get(ppState, i);
            if(passportState == BUSY || passportState == AVAILABLE)
                passportClerksAllOnBreak = FALSE;
            cashierState = Get(cashState, i);
            if(cashierState == BUSY || cashierState == AVAILABLE)
                cashiersAllOnBreak = FALSE;
        }
        
        /* wake up clerks of any type that are all on break*/
        if(pictureClerksAllOnBreak)
        {
            Print("Manager has woken up a PictureClerk\n", 37, 0, 0);
            Acquire(pictureClerkLock);
            /*pictureClerks[0].state = AVAILABLE;*/
            Signal(Get(picBreakCV, 0), Get(picBreakLock, 0)); /*TODO: right now it just wakes up clerk 0*/
            Release(pictureClerkLock);
            pictureClerksAllOnBreak = FALSE;
        }
        if(applicationClerksAllOnBreak)
        {
            Print("Manager has woken up an ApplicationClerk\n", 42, 0, 0);
            Acquire(applicationClerkLock);
            /*applicationClerks[0].state = AVAILABLE;*/
            Signal(Get(appBreakCV, 0), Get(appBreakLock, 0)); /* TODO: right now it just wakes up clerk 0 */
            Release(applicationClerkLock);
            applicationClerksAllOnBreak = FALSE;
        }
        if(passportClerksAllOnBreak)
        {
            Print("Manager has woken up a PassportClerk\n", 38, 0, 0);
            Acquire(passportClerkLock);
            /*passportClerks[0].state = AVAILABLE;*/
            Signal(Get(ppBreakCV, 0), Get(ppBreakLock, 0)); /* TODO: right now it just wakes up clerk 0*/
            Release(passportClerkLock);
            passportClerksAllOnBreak = FALSE;
        }
        if(cashiersAllOnBreak)
        {
            Print("Manager has woken up a Cashier\n", 32, 0, 0);
            Acquire(cashierLock);
            /*cashiers[0].state = AVAILABLE;*/
            Signal(Get(cashBreakCV, 0), Get(cashBreakLock, 0)); /* TODO: right now it just wakes up clerk 0*/
            Release(cashierLock);
            cashiersAllOnBreak = FALSE;
        }
        
        /*if a certain type of clerk has more than 3 customers waiting on them, wake up another clerk of that type*/
        for(i = 0; i < NUM_CLERKS; i++)
        {
            if(signalPictureClerk && Get(picState, i) == ONBREAK)
            {
                Print("Manager has woken up a PictureClerk\n", 37, 0, 0);
                Acquire(pictureClerkLock);
                /*pictureClerks[i].state = AVAILABLE;*/
                Signal(Get(picBreakCV, i), Get(picBreakLock, i)); 
                Release(pictureClerkLock);
                signalPictureClerk = FALSE;
            }
            if(signalAppClerk && Get(appState, i) == ONBREAK)
            {
                Print("Manager has woken up an ApplicationClerk\n", 42, 0, 0);
                Acquire(applicationClerkLock);
                /*applicationClerks[i].state = AVAILABLE;*/
				Signal(Get(appBreakCV, i), Get(appBreakLock, i)); 
                Release(applicationClerkLock);
                signalAppClerk = FALSE;
                
            }
            if(signalPassportClerk && Get(ppState, i) == ONBREAK)
            {
                Print("Manager has woken up a PassportClerk\n", 38, 0, 0);
                Acquire(passportClerkLock);
                /*passportClerks[i].state = AVAILABLE;*/
				Signal(Get(ppBreakCV, i), Get(ppBreakLock, i)); 
                Release(passportClerkLock);
                signalPassportClerk = FALSE;
            }
            if(signalCashier && Get(cashState, i) == ONBREAK)
            {
                Print("Manager has woken up a Cashier\n", 32, 0, 0);
                Acquire(cashierLock);
                /*cashiers[i].state = AVAILABLE;*/
                Signal(Get(cashBreakCV, i), Get(cashBreakLock, i)); 
                Release(cashierLock);
                signalCashier = FALSE;
            }
        }
        
        /*wait a bit to print the revenue*/
        for(i =0;i<400;i++)
        {
            Yield();
        }
        
        pictureRevenue = 0;
        applicationRevenue = 0;
        passportRevenue = 0;
        cashierRevenue = 0;
        
        /*tally up the revenues and print them*/
        for (i = 0; i < NUM_CLERKS; i++)
        {
            pictureRevenue += Get(picMoney, i);
            applicationRevenue += Get(appMoney, i);
            passportRevenue += Get(ppMoney, i);
            cashierRevenue += Get(cashMoney, i);
        }
        
        Print("Manager has counted a total of $%i for PictureClerks\n", 54, pictureRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for ApplicationClerks\n", 58, applicationRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for PassportClerks\n", 55, passportRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for Cashiers\n", 49, cashierRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for the passport office\n", 60,(pictureRevenue+applicationRevenue+passportRevenue+cashierRevenue) * 1000, 0);
        
        
    }
    Print("This passport office has finished\n", 34, 0, 0);
    

    Exit(0);
}
