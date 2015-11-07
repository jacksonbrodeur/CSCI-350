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
    int holder;
    List* waitQueue;
    bool isBusy;

    ServerLock() {
        name = "";
        holder = -1;
        waitQueue = new List;
        isBusy = false; 
    }

    ServerLock(char * _name) {
        name = _name;
        holder = -1;
        waitQueue = new List;
        isBusy = false;
    }

};

vector<ServerLock*> * serverLocks;

int CreateLock(char* name);

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

        switch(rpc) {
            case CREATE_LOCK:
                char* name = new char[MaxMailSize];
                ss >> name;
                ss.clear();
                ss.str("");
                int index = CreateLock(name);
                outPktHdr.to = inPktHdr.from;
                outMailHdr.to = 0;
                ss << SUCCESS << " " << index;
                char data[MaxMailSize];
                strcpy(data, ss.str().c_str());
                outMailHdr.length = strlen(data) + 1;
                bool success = postOffice->Send(outPktHdr, outMailHdr, data);
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

