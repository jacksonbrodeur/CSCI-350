
#include "syscall.h"
#define TRUE 1
#define FALSE 0
#define NULL 0

/* Clerk States */
int AVAILABLE = 0;
int BUSY = 1;
int ONBREAK = 2;

/* Clerk Types */
int PICTURECLERK = 0;
int APPLICATIONCLERK = 1;
int PASSPORTCLERK = 2;
int CASHIER = 3;

/* Clerk Locks */
int applicationClerkLock;
int pictureClerkLock;
int passportClerkLock;
int cashierLock;

/* increment when a clerk/customer is forked */
int counterLock;
int numApplicationClerks = 0;
int numPictureClerks = 0;
int numPassportClerks = 0;
int numCashiers = 0;
int numCustomers = 0;

/* Senator data */
int senatorLock;
int senatorCondition;
int numSenatorsHere = 0;

int totalCustomerMoney = 0;
int storeJustOpened = 0;
int customersFinished = 0;

#define MAX_CUSTOMERS 200
#define MAX_CLERKS 50
int NUM_CUSTOMERS = 40;
int NUM_CLERKS = 5;
int NUM_SENATORS = 3;

typedef struct Customer {
    int id;
    int applicationFiled;
    int pictureTaken;
    int pictureFiled;
    int passportCertified;
    int passportGiven;
    int cashierPaid;
    int money;
    int isSenator;
} Customer;

typedef struct Clerk {
    int myLine;
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
} Clerk;

Customer customers[MAX_CUSTOMERS];
Clerk pictureClerks[MAX_CLERKS];
Clerk applicationClerks[MAX_CLERKS];
Clerk passportClerks[MAX_CLERKS];
Clerk cashiers[MAX_CLERKS];


void initializeData() {
    int i;
    counterLock = CreateLock("counter lock", 12);
    senatorLock = CreateLock("senator lock", 12);
    senatorCondition = CreateCondition("senator condition", 17);
    applicationClerkLock = CreateLock("application clerk lock", 22);
    pictureClerkLock = CreateLock("picture clerk lock", 18);
    passportClerkLock = CreateLock("passport clerk lock", 19);
    cashierLock = CreateLock("cashier lock", 12);

    for (i = 0; i < NUM_CLERKS; i++)
    {
        pictureClerks[i].myLine = i;
        pictureClerks[i].lineCount = 0;
        pictureClerks[i].bribeLineCount = 0;
        pictureClerks[i].state = AVAILABLE;
        pictureClerks[i].lineCondition = CreateCondition("line condition", 14);
        pictureClerks[i].bribeLineCondition = CreateCondition("bribe line condition", 20);
        pictureClerks[i].clerkCondition = CreateCondition("clerk condition", 15);

        pictureClerks[i].breakLock = CreateLock("break lock", 10);
        pictureClerks[i].breakCondition = CreateCondition("break condition", 15);

        pictureClerks[i].clerkLock = CreateLock("clerk lock", 10);
        pictureClerks[i].clerkType = PICTURECLERK;
        pictureClerks[i].customer = NULL;
        pictureClerks[i].money = 0;


        applicationClerks[i].myLine = i;
        applicationClerks[i].lineCount = 0;
        applicationClerks[i].bribeLineCount = 0;
        applicationClerks[i].state = AVAILABLE;
        applicationClerks[i].lineCondition = CreateCondition("line condition", 14);
        applicationClerks[i].bribeLineCondition = CreateCondition("bribe line condition", 20);
        applicationClerks[i].clerkCondition = CreateCondition("clerk condition", 15);

        applicationClerks[i].breakLock = CreateLock("break lock", 10);
        applicationClerks[i].breakCondition = CreateCondition("break condition", 15);

        applicationClerks[i].clerkLock = CreateLock("clerk lock", 10);
        applicationClerks[i].clerkType = APPLICATIONCLERK;
        applicationClerks[i].customer = NULL;
        applicationClerks[i].money = 0;


        passportClerks[i].myLine = i;
        passportClerks[i].lineCount = 0;
        passportClerks[i].bribeLineCount = 0;
        passportClerks[i].state = AVAILABLE;
        passportClerks[i].lineCondition = CreateCondition("line condition", 14);
        passportClerks[i].bribeLineCondition = CreateCondition("bribe line condition", 20);
        passportClerks[i].clerkCondition = CreateCondition("clerk condition", 15);

        passportClerks[i].breakLock = CreateLock("break lock", 10);
        passportClerks[i].breakCondition = CreateCondition("break condition", 15);

        passportClerks[i].clerkLock = CreateLock("clerk lock", 10);
        passportClerks[i].clerkType = PASSPORTCLERK;
        passportClerks[i].customer = NULL;
        passportClerks[i].money = 0;


        cashiers[i].myLine = i;
        cashiers[i].lineCount = 0;
        cashiers[i].bribeLineCount = 0;
        cashiers[i].state = AVAILABLE;
        cashiers[i].lineCondition = CreateCondition("line condition", 14);
        cashiers[i].bribeLineCondition = CreateCondition("bribe line condition", 20);
        cashiers[i].clerkCondition = CreateCondition("clerk condition", 15);

        cashiers[i].breakLock = CreateLock("break lock", 10);
        cashiers[i].breakCondition = CreateCondition("break condition", 15);

        cashiers[i].clerkLock = CreateLock("clerk lock", 10);
        cashiers[i].clerkType = CASHIER;
        cashiers[i].customer = NULL;
        cashiers[i].money = 0;
    }

    for (i = 0; i < NUM_CUSTOMERS + NUM_SENATORS; i++)
    {
        customers[i].id = i;
        customers[i].applicationFiled = FALSE;
        customers[i].pictureTaken = FALSE;
        customers[i].pictureFiled = FALSE;
        customers[i].passportCertified = FALSE;
        customers[i].passportGiven = FALSE;
        customers[i].cashierPaid = FALSE;
        customers[i].isSenator = FALSE;
        if(NUM_SENATORS != 0 && i % ((NUM_CUSTOMERS+NUM_SENATORS)/NUM_SENATORS) == (NUM_CUSTOMERS/NUM_SENATORS)) {
            customers[i].isSenator = TRUE;
        }
        if(customers[i].isSenator) {
            customers[i].money = 100;
        } else {
            customers[i].money = 100 + (Rand() %4) * 500;
        }
        totalCustomerMoney += customers[i].money;
    }
}


int main()
{
    int i;
    Print("Number of Customers = %i\n", 26, NUM_CUSTOMERS * 1000, 0);
    Print("Number of ApplicationClerks = %i\n", 34, NUM_CLERKS * 1000, 0);
    Print("Number of PictureClerks = %i\n", 30, NUM_CLERKS * 1000, 0);
    Print("Number of PassportClerks = %i\n", 31, NUM_CLERKS * 1000, 0);
    Print("Number of Cashiers = %i\n", 25, NUM_CLERKS * 1000, 0);
    Print("Number of Senators = %i\n", 25, NUM_SENATORS * 1000, 0);

    initializeData();

    for (i = 0; i < NUM_CLERKS; i++) {
        Fork(pictureClerk);
    }
    for (i = 0; i < NUM_CLERKS; i++) {
        Fork(applicationClerk);
    }
    for (i = 0; i < NUM_CLERKS; i++) {
        Fork(passportClerk);
    }
    for (i = 0; i < NUM_CLERKS; i++) {
        Fork(cashier);
    }
    for(i = 0; i < NUM_CUSTOMERS + NUM_SENATORS; i++) {
        Fork(customer);
    }

    Fork(manager);

    Exit(0);
}
