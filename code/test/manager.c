#include "syscall.h"

main()
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
    
    int i;
    
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        
        signalPictureClerk=FALSE;
        signalAppClerk=FALSE;
        signalPassportClerk=FALSE;
        signalCashier=FALSE;
        
        pictureClerksAllOnBreak = TRUE;
        applicationClerksAllOnBreak = TRUE;
        passportClerksAllOnBreak = TRUE;
        cashiersAllOnBreak = TRUE;
        
        /*see if 1) there are any clerks with 3 or more customers waiting on them and 2) if all clerks of a certain type are on break*/
        for(i = 0; i< NUM_CLERKS; i++)
        {
            if(pictureClerks[i].lineCount + pictureClerks[i].bribeLineCount >= 3)
                signalPictureClerk=TRUE;
            if(applicationClerks[i].lineCount + applicationClerks[i].bribeLineCount >= 3)
                signalAppClerk=TRUE;
            if(passportClerks[i].lineCount + passportClerks[i].bribeLineCount >=3)
                signalPassportClerk=TRUE;
            if(cashiers[i].lineCount>=3)
                signalCashier=TRUE;
            if(pictureClerks[i].state == BUSY || pictureClerks[i].state == AVAILABLE)
                pictureClerksAllOnBreak = FALSE;
            if(applicationClerks[i].state == BUSY || applicationClerks[i].state == AVAILABLE)
                applicationClerksAllOnBreak = FALSE;
            if(passportClerks[i].state == BUSY || passportClerks[i].state == AVAILABLE)
                passportClerksAllOnBreak = FALSE;
            if(cashiers[i].state == BUSY || cashiers[i].state == AVAILABLE)
                cashiersAllOnBreak = FALSE;
        }
        
        /* wake up clerks of any type that are all on break*/
        if(pictureClerksAllOnBreak)
        {
            Print("Manager has woken up a PictureClerk\n", 37, 0, 0);
            Acquire(pictureClerkLock);
            /*pictureClerks[0].state = AVAILABLE;*/
            Signal(pictureClerks[0].breakCondition, pictureClerks[0].breakLock);
            Release(pictureClerkLock);
            pictureClerksAllOnBreak=FALSE;
        }
        if(applicationClerksAllOnBreak)
        {
            Print("Manager has woken up an ApplicationClerk\n", 42, 0, 0);
            Acquire(applicationClerkLock);
            /*applicationClerks[0].state = AVAILABLE;*/
            Signal(applicationClerks[0].breakCondition, applicationClerks[0].breakLock);
            Release(applicationClerkLock);
            applicationClerksAllOnBreak=FALSE;
        }
        if(passportClerksAllOnBreak)
        {
            Print("Manager has woken up a PassportClerk\n", 38, 0, 0);
            Acquire(passportClerkLock);
            /*passportClerks[0].state = AVAILABLE;*/
            Signal(passportClerks[0].breakCondition, passportClerks[0].breakLock);
            Release(passportClerkLock);
            passportClerksAllOnBreak=FALSE;
        }
        if(cashiersAllOnBreak)
        {
            Print("Manager has woken up a Cashier\n", 32, 0, 0);
            Acquire(cashierLock);
            /*cashiers[0].state = AVAILABLE;*/
            Signal(cashiers[0].breakCondition, cashiers[0].breakLock);
            Release(cashierLock);
            cashiersAllOnBreak = FALSE;
        }
        
        /*if a certain type of clerk has more than 3 customers waiting on them, wake up another clerk of that type*/
        for(i = 0; i < NUM_CLERKS; i++)
        {
            if(signalPictureClerk && pictureClerks[i].state == ONBREAK)
            {
                Print("Manager has woken up a PictureClerk\n", 37, 0, 0);
                Acquire(pictureClerkLock);
                /*pictureClerks[i].state = AVAILABLE;*/
                Signal(pictureClerks[i].breakCondition, pictureClerks[i].breakLock);
                Release(pictureClerkLock);
                signalPictureClerk=FALSE;
            }
            if(signalAppClerk && applicationClerks[i].state == ONBREAK)
            {
                Print("Manager has woken up an ApplicationClerk\n", 42, 0, 0);
                Acquire(applicationClerkLock);
                /*applicationClerks[i].state = AVAILABLE;*/
                Signal(applicationClerks[i].breakCondition, applicationClerks[i].breakLock);
                Release(applicationClerkLock);
                signalAppClerk=FALSE;
                
            }
            if(signalPassportClerk && passportClerks[i].state == ONBREAK)
            {
                Print("Manager has woken up a PassportClerk\n", 38, 0, 0);
                Acquire(passportClerkLock);
                /*passportClerks[i].state = AVAILABLE;*/
                Signal(passportClerks[i].breakCondition, passportClerks[i].breakLock);
                Release(passportClerkLock);
                signalPassportClerk=FALSE;
            }
            if(signalCashier && cashiers[i].state == ONBREAK)
            {
                Print("Manager has woken up a Cashier\n", 32, 0, 0);
                Acquire(cashierLock);
                /*cashiers[i].state = AVAILABLE;*/
                Signal(cashiers[i].breakCondition, cashiers[i].breakLock);
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
            pictureRevenue += pictureClerks[i].money;
            applicationRevenue += applicationClerks[i].money;
            passportRevenue += passportClerks[i].money;
            cashierRevenue += cashiers[i].money;
        }
        
        Print("Manager has counted a total of $%i for PictureClerks\n", 54, pictureRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for ApplicationClerks\n", 58, applicationRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for PassportClerks\n", 55, passportRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for Cashiers\n", 49, cashierRevenue * 1000, 0);
        Print("Manager has counted a total of $%i for the passport office\n", 60,(pictureRevenue+applicationRevenue+passportRevenue+cashierRevenue) * 1000, 0);
        
        
    }
    Print("This passport office has finished\n", 34, 0, 0);
    
    /*delete program data*/
    
    DestroyLock(pictureClerkLock);
    DestroyLock(applicationClerkLock);
    DestroyLock(passportClerkLock);
    DestroyLock(cashierLock);
    
    DestroyLock(senatorLock);
    DestroyCondition(senatorCondition);
    
    /* Destroy Locks and Conditions within each Clerk */
    for(i = 0; i < NUM_CLERKS; i++) {
        DestroyLock(applicationClerks[i].breakLock);
        DestroyLock(applicationClerks[i].clerkLock);
        DestroyCondition(applicationClerks[i].clerkCondition);
        DestroyCondition(applicationClerks[i].breakCondition);
        DestroyCondition(applicationClerks[i].bribeLineCondition);
        DestroyCondition(applicationClerks[i].lineCondition);
        
        DestroyLock(pictureClerks[i].breakLock);
        DestroyLock(pictureClerks[i].clerkLock);
        DestroyCondition(pictureClerks[i].clerkCondition);
        DestroyCondition(pictureClerks[i].breakCondition);
        DestroyCondition(pictureClerks[i].bribeLineCondition);
        DestroyCondition(pictureClerks[i].lineCondition);
        
        DestroyLock(passportClerks[i].breakLock);
        DestroyLock(passportClerks[i].clerkLock);
        DestroyCondition(passportClerks[i].clerkCondition);
        DestroyCondition(passportClerks[i].breakCondition);
        DestroyCondition(passportClerks[i].bribeLineCondition);
        DestroyCondition(passportClerks[i].lineCondition);
        
        DestroyLock(cashiers[i].breakLock);
        DestroyLock(cashiers[i].clerkLock);
        DestroyCondition(cashiers[i].clerkCondition);
        DestroyCondition(cashiers[i].breakCondition);
        DestroyCondition(cashiers[i].bribeLineCondition);
        DestroyCondition(cashiers[i].lineCondition);
    }
    Exit(0);
}
