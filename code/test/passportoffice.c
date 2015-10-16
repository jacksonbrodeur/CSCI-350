
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


void pictureTransaction(Clerk * clerk, Customer * customer) {
    
    Acquire(clerk->clerkLock);
    clerk->customer = customer;
    Signal(clerk->clerkCondition, clerk->clerkLock);
    Print("Customer %i has given SSN to PictureClerk %i\n", 46, customer->id * 1000 + clerk->myLine, 0);

    while(customer->pictureTaken == 0) {

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

void applicationTransaction(Clerk * clerk, Customer * customer) {
    
    Acquire(clerk->clerkLock);
    clerk->customer = customer;
    Print("Customer %i has given SSn to ApplicationClerk %i", 48, customer->id * 1000 + clerk->myLine, 0);

    Signal(clerk->clerkCondition, clerk->clerkLock);

    Wait(clerk->clerkCondition, clerk->clerkLock);

    Release(clerk->clerkLock);
    
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
    Clerk * me = &passportClerks[myLine];
    
    /* On duty while there are still customers who haven't completed process */
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(passportClerkLock);
        
        /* If there is a customer in line signal him to the counter */
         if(me->bribeLineCount > 0) { /* Check bribe line first... */
            Print("Passport Clerk %d has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
            Signal(me->bribeLineCondition, passportClerkLock);
            me->state = BUSY;
        } else if(me->lineCount > 0) { /* then check regular line */
            Print("Passport Clerk %d has signalled a customer to come to their counter\n", 69, myLine * 1000, 0);
            Signal(me->lineCondition, passportClerkLock);
            me->state = BUSY;
        } else {
            
            if(storeJustOpened >= NUM_CLERKS * 4) {
                Print("Passport Clerk %d is going on break\n", 37, myLine * 1000, 0);
                Release(passportClerkLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;
                
                /* Mesa style monitor, wait to be woken up by manager */
                while(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }
                
                Print("Passport Clerk %d is coming off break\n", 39, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(passportClerkLock);
                
                /* Tell the next customer in line that clerk is ready */
                Signal(me->lineCondition, passportClerkLock);
            }
            else {
                storeJustOpened++;
            }
        }

        /* entering transaction so switch locks */
        Acquire(me->clerkLock);
        Release(passportClerkLock);

        /* wait for customer data */
        Wait(me->clerkCondition, me->clerkLock);
        

        Print("Passport Clerk %d has received SSN from Customer %d\n", 53, myLine * 1000 + me->customer->id, 0);
        Print("Passport Clerk %d has determined that Customer %d does have both their application and picture completed\n", 106, myLine * 1000 + me->customer->id, 0);
        for(int i = 0; i < Rand()% 80 + 20; i++) {
            Yield(currentThread);
        }
        me->customer->passportCertified = true;
        Print("Passport Clerk %d has recorded Customer %d passport documentation\n", 67, myLine * 1000 + me->customer->id, 0);
        Signal(me->clerkCondition, me->clerkLock);
        Release(me->clerkLock);
    }
}

void cashierTransaction() {
    
    
}

void cashier() {
    Clerk * me = &cashiers[myLine];
    
    /* On duty while there are still customers who haven't completed process */
    while(customersFinished < NUM_CUSTOMERS + NUM_SENATORS) {
        Acquire(cashierLock);

        if (me->lineCount > 0) {
            Print("Cashier %d has signalled a customer to come to their counter\n", 62, myLine * 1000, 0);
            Signal(me->lineCondition, cashierLock);
            me->state = BUSY;
        } else {
            if(storeJustOpened>=NUM_CLERKS*4) {
                Print("Cashier %d is going on break\n", 30, myLine * 1000, 0);
                Release(cashierLock);
                Acquire(me->breakLock);
                me->state = ONBREAK;
                
                /* Mesa style monitor, wait to be woken up by manager */
                while(me->state == ONBREAK) {
                    Wait(me->breakCondition, me->breakLock);
                }
                
                Print("Cashier %d is coming off break\n", 32, myLine * 1000, 0);
                Release(me->breakLock);
                Acquire(cashierLock);
                
                Signal(me->lineCondition, cashierLock);
            }
            else {
                storeJustOpened++;
            }
        }

        /* entering transaction so switch locks */
        Acquire(me->clerkLock);
        Release(cashierLock);

        /* wait for customer to pay */
        Wait(me->clerkCondition, me->clerkLock);

        Print("Cashier %d has received SSN from Customer %d\n", 46, myLine * 1000 + me->customer->id, 0);
        Print("Cashier %d has verified that Customer %d has been certified by a PassportClerk\n", 80, myLine * 1000 + me->customer->id, 0);
        /* taking payment */
        me->customer->money -= 100;
        me->money += 100;
        Print("Cashier %d has received the $100 from Customer %d after certification\n", 71, myLine * 1000 + me->customer->id, 0);
        
        me->customer->passportGiven = true;
        me->clerkCondition->Signal(me->clerkLock);
        Print("Cashier %d has provided Customer %d their completed passport\n", 62, myLine * 1000 + me->customer->id, 0);
        Print("Cashier %d has recorded that Customer %d has been given their completed passport\n", 82, myLine * 1000 + me->customer->id, 0);
        
        Release(me->clerkLock);
    }   
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
