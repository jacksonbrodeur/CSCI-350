// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <deque>
#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "list.h"

using namespace std;

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

struct ServerLock {
    char* name;
    int owner;
    deque<int>* waitQueue;
    bool isBusy;

    ServerLock() {
        name = "";
        owner = -1;
        waitQueue = new deque<int>;
        isBusy = false; 
    }

    ServerLock(char * _name) {
        name = _name;
        owner = -1;
        waitQueue = new deque<int>;
        isBusy = false;
    }

};

vector<ServerLock*> * serverLocks;

int CreateLock(char* name);
bool AcquireLock(int index, int machineID);
bool ReleaseLock(int index, int machineID);

void RunServer()
{
    serverLocks = new vector<ServerLock*>;
    printf("Nachos Server Program has started\n\n");
    while (true) {
        
        char buffer[MaxMailSize] = "";
        PacketHeader outPktHdr;
        PacketHeader inPktHdr;
        MailHeader outMailHdr;
        MailHeader inMailHdr;
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);

        printf("%s\n", buffer);
        stringstream ss;
        ss << buffer;
        int rpc;
        ss >> rpc;

        int incomingMachineID = inMailHdr.from;
        int index;
        char data[MaxMailSize];
        bool success;
        switch(rpc) {
            case CREATE_LOCK:
                char* name = new char[MaxMailSize];
                ss >> name;
                ss.clear();
                ss.str("");
                index = CreateLock(name);
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = 0;
                ss << SUCCESS << " " << index;
                strcpy(data, ss.str().c_str());
                outMailHdr.length = strlen(data) + 1;
                success = postOffice->Send(outPktHdr, outMailHdr, data);
                printf("Created lock named %s from machine %d\n", name, incomingMachineID);
                break;
            case ACQUIRE:
                ss >> index;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = 0;
                if(index < 0 || static_cast<uint64_t>(index) > serverLocks->size() - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }
                success = AcquireLock(index, incomingMachineID);
                if(success) {
                    //Send response to incomingMachineID
                    ss << SUCCESS;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    printf("Acquired lock %d from machine %d\n", index, incomingMachineID);
                } else {//else lock is busy, so don't send response
                    printf("Lock %d is busy so machine %d will wait\n", index, incomingMachineID);
                }
                break;
            case RELEASE:
                ss >> index;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                inMailHdr.to = 0;
                if(index < 0 || static_cast<uint64_t>(index) > serverLocks->size() - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }
                printf("Releasing lock %d from machine %d\n", index, incomingMachineID);
                success = ReleaseLock(index, incomingMachineID);
                ss << SUCCESS;
                strcpy(data, ss.str().c_str());
                outMailHdr.length = strlen(data) + 1;
                postOffice->Send(outPktHdr, outMailHdr, data);
                break;

        }
    }
}

int CreateLock(char* name) {
    ServerLock * newLock = new ServerLock(name);
    int index = serverLocks->size();
    serverLocks->push_back(newLock);
    return index;
}

bool AcquireLock(int index, int machineID) {
    ServerLock * sl = serverLocks->at(index);
    bool returnValue = true;
    if(sl->owner == -1) {
        sl->owner = machineID;
        sl->isBusy = true;
    } else if(sl->owner == machineID) {
        //Do nothing
    } else if (sl->isBusy) {
        sl->waitQueue->push_back(machineID);
        returnValue = false;
    }

    return returnValue;
}

bool ReleaseLock(int index, int machineID) {
    ServerLock * sl = serverLocks->at(index);
    if(sl->owner == machineID) {
        if(sl->waitQueue->empty()) {
            sl->isBusy = false;
            sl->owner = -1;
        }
        else {
            int newOwner = sl->waitQueue->front();
            sl->waitQueue->pop_front();
            sl->owner = newOwner;
            stringstream ss;
            PacketHeader outPktHdr;
            MailHeader outMailHdr;

            outPktHdr.to = newOwner;
            outMailHdr.to = 0;
            ss << SUCCESS;
            char data[MaxMailSize];
            strcpy(data, ss.str().c_str());
            outMailHdr.length = strlen(data) + 1;
            postOffice->Send(outPktHdr, outMailHdr, data);
            printf("Machine %d is now the owner of lock %d\n", newOwner, index);
        }
    } else {
        printf("Machine %d did not own lock %d so nothing happens\n", machineID, index);
    }
    return true;
}

