// setup.h 
// used to set up all the resources needed for the passport office
// call this first from the other user programs (i.e. customer.c)

#include "syscall.h"

#define TRUE 1
#define FALSE 0
#define NULL 0

#define MAX_CUSTOMERS 200
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

// should we #define these ^^ ?

int NUM_CUSTOMERS;
int NUM_CLERKS;
int NUM_SENATORS;

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

int customers;
int pictureClerks;
int applicationClerks;
int passportClerks;
int cashiers;


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
    // TODO: update sets here (pass singleton array of 0)
    numApplicationClerks = CreateMV("numAppClerks", 12);
    Set(numApplicationClerks, 0);
    numPictureClerks = CreateMV("numPicClerks", 12);
    Set(numPicClerks, 0);
    numPassportClerks = CreateMV("numPassportClerks", 17);
    Set(numPassportClerks, 0);
    numCashiers = CreateMV("numCashiers", 11);
    Set(numCashiers, 0);
    numCustomers = CreateMV("numCustomers", 12);
    Set(numCustomers, 0);
    numSenatorsHere = CreateMV("numSenatorsHere", 15);
    Set(numSenatorsHere, 0);
    totalCustomerMoney = CreateMV("totalCustMoney", 14);
    Set(totalCustMoney, 0);
    storeJustOpened = CreateMV("storeJustOpened", 15);
    Set(storeJustOpened, 0);
    customersFinished = CreateMV("customersFinished", 17);
    Set(customersFinished, 0);
    NUM_CUSTOMERS = CreateMV("NUM_CUSTOMERS", 13);
    NUM_CLERKS = CreateMV("NUM_CLERKS", 10);
    NUM_SENATORS = CreateMV("NUM_SENATORS", 12);

    // entity arrays
 	customers = CreateMV("customers", 9);
 	pictureClerks = CreateMV("pictureClerks", 13);
 	applicationClerks = CreateMV("applicationClerks", 17);
 	passportClerks = CreateMV("passportClerks", 14);
 	cashiers = CreateMV("cashiers", 8);
 	// TODO: set them as arrays with MAX_CUSTOMERS / MAX_CLERKS
}