
#include "syscall.h"
int AVAILABLE = 0;
int BUSY = 1;
int ONBREAK = 2;

int PICTURECLERK = 0;
int APPLICATIONCLERK = 1;
int PASSPORTCLERK = 2;
int CASHIER = 3;

int applicationClerkLock;
int pictureClerkLock;
int passportClerkLock;
int cashierLock;

Customer customers[150];
Clerk pictureClerks[50];
Clerk applicationClerks[50];
Clerk passportClerks[50];
Clerk cashiers[50];

struct Clerk {
    int myLine;
    int lineCount;
    int bribeLineCount;
    int state
    int lineCondition;
    int bribeLineCondition
    int clerkCondition

    int breakLock;
    int breakCondition;

    int clerkLock;
    int clerkType;
    Customer * customer;
    int money;
};

struct Customer {
    int id;
    int applicationFiled;
    int pictureTaken;
    int pictureFiled;
    int passportCertified;
    int passportGiven;
    int cashierPaid;
    int money;
    int isSenator;
};
int totalCustomerMoney;

struct Clerk createClerk(int line, int type) {
    int myLine = line;
    int lineCount = 0;
    int bribeLineCount = 0;
    int state = 0;
    int lineCondition = CreateCondition("line condition", 14);
    int bribeLineCondition = CreateCondition("bribe line condition", 20);
    int clerkCondition = CreateCondition("clerk condition", 15);

    int breakLock = CreateLock("break lock", 10);
    int breakCondition = CreateCondition("break condition", 15);

    int clerkLock = CreateLock("clerk lock", 10);
    int clerkType = type;
    Customer * customer = NULL;
    int money = 0;

    struct Clerk clerk = {myLine, lineCount, bribeLineCount, 
                            state, lineCondition, bribeLineCondition,
                            lineCondition, bribeLineCondition, clerkCondition,
                            breakLock, breakCondition, clerkLock, 
                            clerkType, customer, money};

    return clerk;
}

struct Customer createCustomer(int id, int senator) {
    int myId = id;
    int applicationFiled = 0;
    int pictureTaken = 0;
    int pictureFiled = 0;
    int passportCertified = 0;
    int passportGiven = 0;
    int cashierPaid = 0;
    int isSenator = senator;
    int money;
    if(isSenator == 1) {
        money = 100;
    } else {
        money = 100 + (Rand() % 4) * 500;
    }
    totalCustomerMoney += money;

    struct Customer customer = {myId, applicationFiled, pictureTaken,
                                pictureFiled, passportCertified, passportGiven,
                                cashierPaid, money, isSenator};

    return customer;
}


void pictureTransaction() {
    
    
}

void pictureClerk() {
    int firstTime = 1;
    int i;
    int j = Rand() % 80 + 20;
    Clerk * me = &pictureClerks[myLine];

    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(pictureClerkLock);

        /* If there is a customer in line signal him to the counter */
        if(me->bribeLineCount > 0) {
            Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->lineCondition, pictureClerkLock);
            me->state = BUSY;
        } else if(me->lineCount > 0) {
            Print("PictureClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->lineCondition, pictureClerkLock);
            me->state = BUSY;
        } else {

            if(storeJustOpened >= NUM_CLERKS * 4) {
                Print("PictureClerk %i is going on break\n", 35, myLine * 1000, 0);
                Release(pictureClerkLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;

                while(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }

                Print("PictureClerk %i is coming off break\n", 37, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(pictureClerkLock);

                Signal(my->lineCondition, pictureClerkLock);
            } else {
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
            Wait(me->clerkCondition, clerkLock);
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

}

void applicationTransaction() {
    
    
}

void applicationClerk() {
    int i;
    int j = Rand() % 80 + 20;
    Clerk * me = &applicationClerks[myLine];
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {

        Acquire(applicationClerkLock);

        /* If there is a customer in line signal him to the counter */
        if(me->bribeLineCount > 0) {
            Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->lineCondition, applicationClerkLock);
            me->state = BUSY;
        } else if(me->lineCount > 0) {
            Print("ApplicationClerk %i has signalled a customer to come to their counter\n", 67, myLine * 1000, 0);
            Signal(me->lineCondition, applicationClerkLock);
            me->state = BUSY;
        } else {

            if(storeJustOpened >= NUM_CLERKS * 4) {
                Print("ApplicationClerk %i is going on break\n", 35, myLine * 1000, 0);
                Release(applicationClerkLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;

                while(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }

                Print("ApplicationClerk %i is coming off break\n", 37, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(applicationClerkLock);

                Signal(my->lineCondition, applicationClerkLock);
            } else {
                storeJustOpened++;
            }
        }

        Acquire(me->clerkLock);
        Release(applicationClerkLock);

        Wait(me->clerkCondition, me->clerkLock);

        Print("ApplicationClerk %i has received SSN from Customer %i\n", 55, myLine * 1000 + me->customer->id, 0);
        for(i = 0; i < j; i++) {
            Yield();
        }
        me->customer->applicationField = 1;
        Print("ApplicationClerk %i has recorded a completed application for Customer %i\n", 74, myLine * 1000 + me->customer->id, 0);

        Signal(me->clerkCondition, me->clerkLock);

        Release(me->clerkLock);
    }
}

void passportTransaction() {
    
   
}

void passportClerk() {
    

}

void cashierTransaction() {
    
    
}

void cashier() {
    
  
}

int getInShortestLine() {
    
}

void customer() {
    
}

void manager() {
    
}



int main()
{
    Yield();
}
