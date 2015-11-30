// setup.h 
// used to set up all the resources needed for the passport office
// call this first from the other user programs (i.e. customer.c)

#include "syscall.h"

#define TRUE 1
#define FALSE 0
#define NULL 0

#define MAX_CUSTOMERS 50
#define MAX_CLERKS 50

/* Clerk States */
#define AVAILABLE 0
#define BUSY 1
#define ONBREAK 2

/* Clerk Types */
#define PICTURECLERK 0
#define APPLICATIONCLERK 1
#define PASSPORTCLERK 2
#define CASHIER 3

#define NUM_CUSTOMERS 50
#define NUM_CLERKS 5
#define NUM_SENATORS 3

/* Clerk Locks */
int applicationClerkLock;
int pictureClerkLock;
int passportClerkLock;
int cashierLock;

/* increment when a clerk/customer is forked */
int counterLock;
int numApplicationClerks;
int numPictureClerks;
int numPassportClerks;
int numCashiers;
int numCustomers;

/* Senator data */
int senatorLock;
int senatorCondition;
int numSenatorsHere;

int totalCustomerMoney;
int storeJustOpened;
int customersFinished;


// Customer struct
//////////////////
int customerIDs; // changed from "id"
int applicationFiled;
int pictureTaken;
int pictureFiled;
int passportCertified;
int passportGiven;
int cashierPaid;
int custMoney; // changed from "money"
int isSenator;

// Clerk struct
///////////////
// picture clerk
int picLine; 
int picLineCount; 
int picBribeLineCount;
int picState;
int picLineCV;
int picBribeLineCV;
int picClerkCV;

int picBreakLock;
int picBreakCV;

int picClerkLock;
int picClerkType;
int picCustomer;
int picMoney;
// application clerk
int appLine; 
int appLineCount; 
int appBribeLineCount;
int appState;
int appLineCV;
int appBribeLineCV;
int appClerkCV;

int appBreakLock;
int appBreakCV;

int appClerkLock;
int appClerkType;
int appCustomer;
int appMoney;
// passport clerk
int ppLine; 
int ppLineCount; 
int ppBribeLineCount;
int ppState;
int ppLineCV;
int ppBribeLineCV;
int ppClerkCV;

int ppBreakLock;
int ppBreakCV;

int ppClerkLock;
int ppClerkType;
int ppCustomer;
int ppMoney;
// cashier
int cashLine; 
int cashLineCount; 
int cashBribeLineCount;
int cashState;
int cashLineCV;
int cashBribeLineCV;
int cashClerkCV;

int cashBreakLock;
int cashBreakCV;

int cashClerkLock;
int cashClerkType;
int cashCustomer;
int cashMoney;




void setup()
{
	// locks
	counterLock = CreateLock("counterLock", 11);
    senatorLock = CreateLock("senatorLock", 11);
    applicationClerkLock = CreateLock("applicationLock", 15);
    pictureClerkLock = CreateLock("pictureLock", 11);
    passportClerkLock = CreateLock("passportLock", 12);
    cashierLock = CreateLock("cashierLock", 11);
    
    // CVs
    senatorCondition = CreateCondition("senatorCV", 9);

    // MVs (shared data)
    numApplicationClerks = CreateMV("numAppClerks", 12);
    Set(numApplicationClerks, 0, 0);
    numPictureClerks = CreateMV("numPicClerks", 12);
    Set(numPicClerks, 0, 0);
    numPassportClerks = CreateMV("numPassportClerks", 17);
    Set(numPassportClerks, 0, 0);
    numCashiers = CreateMV("numCashiers", 11);
    Set(numCashiers, 0, 0);
    numCustomers = CreateMV("numCustomers", 12);
    Set(numCustomers, 0, 0);
    numSenatorsHere = CreateMV("numSenatorsHere", 15);
    Set(numSenatorsHere, 0, 0);
    totalCustomerMoney = CreateMV("totalCustMoney", 14);
    Set(totalCustMoney, 0, 0);
    storeJustOpened = CreateMV("storeJustOpened", 15);
    Set(storeJustOpened, 0, 0);
    customersFinished = CreateMV("customersFinished", 17);
    Set(customersFinished, 0, 0);


 	// TODO: should MAX_CUSTOMERS be 200 and not 50 (what he said we'll use)?

    // customer
    customerIDs = CreateMV("customerIDs", 11); 
    applicationFiled = CreateMV("applicationFiled", 16);
    pictureTaken = CreateMV("pictureTaken", 12);
    pictureFiled = CreateMV("pictureFiled", 12);
    passportCertified = CreateMV("passportCertified", 17);
    passportGiven = CreateMV("passportGiven", 13);
    cashierPaid = CreateMV("cashierPaid", 11);
    custMoney = CreateMV("custMoney", 9); 
    isSenator = CreateMV("isSenator", 9);


    // picture clerk
    picLine = CreateMV("picLine", 7);
    picLineCount = CreateMV("picLineCount", 12);
    picBribeLineCount = CreateMV("picBribeLineCount", 17);
    picState = CreateMV("picState", 8);
    picLineCV = CreateMV("picLineCV", 9);
    picBribeLineCV = CreateMV("picBribeLineCV", 14);
    picClerkCV = CreateMV("picClerkCV", 10);

    picBreakLock = CreateMV("picBreakLock", 12);
    picBreakCV = CreateMV("picBreakCV", 10);

    picClerkLock = CreateMV("picClerkLock", 12);
    picClerkType = CreateMV("picClerkType", 12);
    picCustomer = CreateMV("picCustomer", 11);
    picMoney = CreateMV("picMoney", 8);

    // application clerk
    appLine = CreateMV("appLine", 7);
    appLineCount = CreateMV("appLineCount", 12);
    appBribeLineCount = CreateMV("appBribeLineCount", 17);
    appState = CreateMV("appState", 8);
    appLineCV = CreateMV("appLineCV", 9);
    appBribeLineCV = CreateMV("appBribeLineCV", 14);
    appClerkCV = CreateMV("appClerkCV", 10);

    appBreakLock = CreateMV("appBreakLock", 12);
    appBreakCV = CreateMV("appBreakCV", 10);

    appClerkLock = CreateMV("appClerkLock", 12);
    appClerkType = CreateMV("appClerkType", 12);
    appCustomer = CreateMV("appCustomer", 11);
    appMoney = CreateMV("appMoney", 8);

    // passport clerk
    ppLine = CreateMV("ppLine", 6);
    ppLineCount = CreateMV("ppLineCount", 11);
    ppBribeLineCount = CreateMV("ppBribeLineCount", 16);
    ppState = CreateMV("ppState", 7);
    ppLineCV = CreateMV("ppLineCV", 8);
    ppBribeLineCV = CreateMV("ppBribeLineCV", 13);
    ppClerkCV = CreateMV("ppClerkCV", 9);

    ppBreakLock = CreateMV("ppBreakLock", 11);
    ppBreakCV = CreateMV("ppBreakCV", 9);

    ppClerkLock = CreateMV("ppClerkLock", 11);
    ppClerkType = CreateMV("ppClerkType", 11);
    ppCustomer = CreateMV("ppCustomer", 10);
    ppMoney = CreateMV("ppMoney", 7);

    // cashier
    cashLine = CreateMV("cashLine", 8);
    cashLineCount = CreateMV("cashLineCount", 13);
    cashBribeLineCount = CreateMV("cashBribeLineCount", 18);
    cashState = CreateMV("cashState", 9);
    cashLineCV = CreateMV("cashLineCV", 10);
    cashBribeLineCV = CreateMV("cashBribeLineCV", 15);
    cashClerkCV = CreateMV("cashClerkCV", 11);

    cashBreakLock = CreateMV("cashBreakLock", 13);
    cashBreakCV = CreateMV("cashBreakCV", 11);

    cashClerkLock = CreateMV("cashClerkLock", 13);
    cashClerkType = CreateMV("cashClerkType", 13);
    cashCustomer = CreateMV("cashCustomer", 12);
    cashMoney = CreateMV("cashMoney", 9);
}