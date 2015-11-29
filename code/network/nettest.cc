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
#include <map>
#include <utility>
#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "list.h"

using namespace std;

#define MAX_RESOURCES 100
#define MAX_MV 50

#define NO          0
#define YES         1
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
    pair<int, int> owner;
    deque<pair<int, int> >* waitQueue;
    bool isBusy;

    ServerLock() {
        name = "";
        owner = make_pair(-1, 0);
        waitQueue = new deque<pair<int,int> >;
        isBusy = false; 
    }

    ServerLock(char * _name) {
        name = _name;
        owner = make_pair(-1, 0);
        waitQueue = new deque<pair<int, int> >;
        isBusy = false;
    }

};

struct ServerCV {
    char * name;
    int waitingLock;
    deque<int>* cvWaitQueue;

    ServerCV() {
        name = "";
        waitingLock = -1;
        cvWaitQueue = new deque<int>;
    }

    ServerCV(char * _name) {
        name = _name;
        waitingLock = -1;
        cvWaitQueue = new deque<int>;
    }
};

struct ServerMV {
    char * name;
    int value[MAX_MV];

    ServerMV() {
        name = "";
        for (int i = 0; i < MAX_MV; i++)
        	value[i] = 0;
    }

    ServerMV(char* _name) {
        name = _name;
        for (int i = 0; i < MAX_MV; i++)
        	value[i] = 0;
    }
};

struct Request {
    int rpcType;
    char * name;
    int index;
    int lockIndex;
    int conditionIndex;
    int value;
    int noCounter;
    int fromMachineID;
    int fromMailbox;

};

vector<ServerLock*> * serverLocks;
vector<ServerCV*> * serverCVs;
vector<ServerMV*> * serverMVs;

map<int, Request*> requestTable; //Map Request ID to number of No's received

int requestCounter = 0;
int lockCounter = 0;
int cvCounter = 0;
int mvCounter = 0;

bool haveSameCreate(char* name);
void TakeAction(int requestID);

int FindLock(char* name);
int CreateLock(char* name);
bool AcquireLock(int index, int machineID, int mailbox);
bool ReleaseLock(int index, int machineID, int mailbox);

int FindCV(char* name);
int CreateCV(char* name);
bool Wait(int conditionIndex, int lockIndex, int machineID);
void Signal(int conditionIndex, int lockIndex);
void Broadcast(int conditionIndex, int lockIndex);

int FindMV(char* name);
int CreateMV(char* name);
void SetMV(int serverIndex, int mvIndex, int value);
int GetMV(int serverIndex, int mvIndex);

void RunServer()
{
    serverLocks = new vector<ServerLock*>;
    serverCVs = new vector<ServerCV*>;
    serverMVs = new vector<ServerMV*>;
    lockCounter += MAX_RESOURCES * myMachineID; // number resources to avoid having to reference by name 
    cvCounter += MAX_RESOURCES * myMachineID; // when checking (take into account server - look for index 15 on server 2 request 215)
    mvCounter += MAX_RESOURCES * myMachineID;
    printf("Nachos Server Program has started\n\n");
    while (true) {
        
        char buffer[MaxMailSize] = "";
        PacketHeader outPktHdr;
        PacketHeader inPktHdr;
        MailHeader outMailHdr;
        MailHeader inMailHdr;
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);

        printf("Message from Machine [%d:%d]: %s\n", inPktHdr.from, inMailHdr.from, buffer);
        stringstream ss;
        ss << buffer;
        int rpc;
        ss >> rpc;

        int incomingMachineID = inPktHdr.from;
        int incomingMailbox = inMailHdr.from;
        int index;
        int mvIndex;
        char data[MaxMailSize];
        bool success;
        char* name = new char[MaxMailSize];
        int conditionIndex;
        int lockIndex;
        int value;
        int requestID;
        int machineID;
        int mailbox;
        switch(rpc) {
            case S_RESPONSE:
                int response;
                ss >> response;
                ss >> requestID;
                if(response == NO) {
                    if (requestTable.count(requestID) > 0) {
                        printf("Received a NO from server %d regarding request %d\n", incomingMachineID, requestID);
                        requestTable[requestID]->noCounter++;
                        if(requestTable[requestID]->noCounter == NUM_SERVERS - 1) {
                            TakeAction(requestID);
                        }
                    } // else: request does not exist in table (request has been resolved) so do nothing
                } else { 
                    // it's a yes so we will indicate that this request is closed by removing it 
                    requestTable.erase(requestID);
                }
                break;
            case S_CREATE_LOCK:
                ss >> name;
                ss >> requestID;
                ss >> machineID;
                ss >> mailbox;
                ss.clear();
                ss.str("");
                index = FindLock(name);
                if(index == -1) {
                    if (haveSameCreate(name) && (myMachineID < incomingMachineID)) {
                        printf("Have same pending request for lock %s, sending YES to server %d because my machine ID is lower\n", name, incomingMachineID);
                        ss << S_RESPONSE << " " << YES << " " << requestID;
                    } else {
                        printf("Did not find lock %s, sending NO to server %d\n", name, incomingMachineID);
                        ss << S_RESPONSE << " " << NO << " " << requestID;
                    } 
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.to = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);                 
                } else {
                    printf("Found lock %s, messaging client [%d:%d] and sending YES to server %d\n", name, machineID, mailbox, incomingMachineID);
                    // Send reply to client with lock index
                    outPktHdr.to = machineID;
                    outMailHdr.to = mailbox;
                    ss << SUCCESS << " " << index;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);

                    ss.clear();
                    ss.str("");

                    //Send yes to other serve
                    ss << S_RESPONSE << " " << YES << " " << requestID;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                }
                break;
            case CREATE_LOCK:
                name = new char[MaxMailSize];
                ss >> name;
                ss.clear();
                ss.str("");
                index = FindLock(name);
                if(index == -1) {
                    printf("Did not find lock %s\n", name);
                    requestID = requestCounter;
                    requestCounter++;
                    Request * r = new Request();
                    r->rpcType = rpc;
                    r->name = name;
                    r->noCounter = 0;
                    r->fromMachineID = incomingMachineID;
                    r->fromMailbox = incomingMailbox;
                    requestTable[requestID] = r;
                    //You don't have lock
                    //Ask other servers
                    for (int i = 0; i < NUM_SERVERS; i++)
                    {
                        if(i == myMachineID) {
                            continue;
                        }
                        printf("Sending message to Server %d to check CREATE_LOCK\n", i);
                        outPktHdr.to = i;
                        outPktHdr.from = myMachineID;
                        outMailHdr.to = 0;
                        outMailHdr.from = 0;
                        ss << S_CREATE_LOCK << " " << name << " " << requestID << " " << incomingMachineID << " " << incomingMailbox;
                        strcpy(data, ss.str().c_str());
                        ss.clear();
                        ss.str("");
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr,outMailHdr, data);
                    }
                } else {
                    outPktHdr.to = incomingMachineID;
                    outMailHdr.to = incomingMailbox;
                    ss << SUCCESS << " " << index;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, data);
                    printf("Retrieved lock named %s from machine %d\n", name, incomingMachineID);
                }
                break;
            case S_ACQUIRE:
                ss >> index;
                ss >> requestID;
                ss >> machineID;
                ss >> mailbox;
                ss.clear();
                ss.str("");
                // see if i have it 
                if(index < (myMachineID * MAX_RESOURCES) || index > ((myMachineID + 1) * MAX_RESOURCES) - 1) {
                    printf("Did not find lock %d, sending NO to server %d\n", index, incomingMachineID);
                    ss << S_RESPONSE << " " << NO << " " << requestID;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.from = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                } else { // I should have it (it's in my range)
                    if (index % MAX_RESOURCES > serverLocks->size()) {
                        // in my range but invalid
                        ss << ERROR;
                        strcpy(data, ss.str().c_str());
                        outPktHdr.to = machineID;
                        outMailHdr.to = mailbox;
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr, outMailHdr, data);
                        break;
                    }
                    success = AcquireLock(index % MAX_RESOURCES, machineID, mailbox);
                    if(success) {
                        //Send response to machineID
                        ss << SUCCESS;
                        strcpy(data, ss.str().c_str());
                        outPktHdr.to = machineID;
                        outMailHdr.to = mailbox;
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr, outMailHdr, data);
                        printf("Acquired lock %d from machine %d\n", index, incomingMachineID);
                    } else {//else lock is busy, so don't send response
                        printf("Lock %d is busy so machine %d will wait\n", index, incomingMachineID);
                    }
                    ss.clear();
                    ss.str("");

                    //Send yes to other server
                    ss << S_RESPONSE << " " << YES << " " << requestID;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.from = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                }
            	break;
            case ACQUIRE:
                ss >> index;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                if(index < 0 || static_cast<uint64_t>(index) > (MAX_RESOURCES * NUM_SERVERS) - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.to = incomingMailbox;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }
                // see if i have it and then send request to servers if I don't
                if(index < (myMachineID * MAX_RESOURCES) || index > ((myMachineID + 1) * MAX_RESOURCES) - 1) {
                    printf("Did not find lock %d\n", index);
                    requestID = requestCounter;
                    requestCounter++;
                    Request * r = new Request();
                    r->rpcType = rpc;
                    r->noCounter = 0;
                    r->fromMachineID = incomingMachineID;
                    r->fromMailbox = incomingMailbox;
                    requestTable[requestID] = r;
                    //You don't have lock
                    //Ask other servers
                    for (int i = 0; i < NUM_SERVERS; i++)
                    {
                        if(i == myMachineID) {
                            continue;
                        }
                        printf("Sending message to Server %d to check ACQUIRE\n", i);
                        outPktHdr.to = i;
                        outPktHdr.from = myMachineID;
                        outMailHdr.to = 0;
                        outMailHdr.from = 0;
                        ss << S_ACQUIRE << " " << index << " " << requestID << " " << incomingMachineID << " " << incomingMailbox;
                        strcpy(data, ss.str().c_str());
                        ss.clear();
                        ss.str("");
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr,outMailHdr, data);
                    }
                } else { // I should have it (it's in my range)
                    if (index % MAX_RESOURCES > serverLocks->size()) {
                        // in my range but invalid
                        ss << ERROR;
                        strcpy(data, ss.str().c_str());
                        outMailHdr.to = incomingMailbox;
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr, outMailHdr, data);
                        break;
                    }
                    success = AcquireLock(index % MAX_RESOURCES, incomingMachineID, incomingMailbox);
                    if(success) {
                        //Send response to incomingMachineID
                        ss << SUCCESS;
                        strcpy(data, ss.str().c_str());
                        outMailHdr.to = incomingMailbox;
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr, outMailHdr, data);
                        printf("Acquired lock %d from machine %d\n", index, incomingMachineID);
                    } else {//else lock is busy, so don't send response
                        printf("Lock %d is busy so machine %d will wait\n", index, incomingMachineID);
                    }
                }                
                break;
            case S_RELEASE:
                ss >> index;
                ss >> requestID;
                ss >> machineID;
                ss >> mailbox;
                ss.clear();
                ss.str("");
                if(index < (myMachineID * MAX_RESOURCES) || index > (myMachineID + 1) * MAX_RESOURCES - 1) {
                    printf("Did not find lock %d, sending NO to server %d\n", index, incomingMachineID);
                    ss << S_RESPONSE << " " << NO << " " << requestID;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.from = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);             
                } else {
                    if(index % MAX_RESOURCES > serverLocks->size()) {
                        //Lock is in my range but does not exist
                        ss << ERROR;
                        strcpy(data, ss.str().c_str());
                        outPktHdr.to = machineID;
                        outMailHdr.to = mailbox;
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr, outMailHdr, data);
                        break;
                    }
                    success = ReleaseLock(index % MAX_RESOURCES, machineID, mailbox);
                    //success should always be true;
                    ss << SUCCESS;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = machineID;
                    outMailHdr.to = mailbox;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    printf("Released lock %d from machine %d\n", index, incomingMachineID);

                    ss.clear();
                    ss.str("");
                    
                    //Send yes to the original server
                    ss << S_RESPONSE << " " << YES << " " << requestID;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.from = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                }
                break;
            case RELEASE:
                ss >> index;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = incomingMailbox;
                if(index < 0 || static_cast<uint64_t>(index) > (MAX_RESOURCES * NUM_SERVERS) - 1) {
                    //Index is out of range for all servers
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }
                if(index < myMachineID * MAX_RESOURCES || index > (myMachineID + 1) * MAX_RESOURCES - 1) {
                    //Lock is not located on this server but might be on another
                    printf("Did not find lock %d\n", index);
                    //Store request so that some of its state is saved for when you get 'no's from other servers
                    requestID = requestCounter;
                    requestCounter++;
                    Request * r = new Request();
                    r->rpcType = rpc;
                    r->noCounter = 0;
                    r->fromMachineID = incomingMachineID;
                    r->fromMailbox = incomingMailbox;
                    requestTable[requestID] = r;

                    //Ask other servers to do the release
                    for (int i = 0; i < NUM_SERVERS; i++)
                    {
                        if(i == myMachineID) {
                            continue;
                        }
                        printf("Sending message to Server %d to check ACQUIRE\n", i);
                        outPktHdr.to = i;
                        outPktHdr.from = myMachineID;
                        outMailHdr.to = 0;
                        outMailHdr.from = 0;
                        ss << S_RELEASE << " " << index << " " << requestID << " " << incomingMachineID << " " << incomingMailbox;
                        strcpy(data, ss.str().c_str());
                        ss.clear();
                        ss.str("");
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr,outMailHdr, data);
                    }

                } else {
                    //I should have it because it is in my range
                    if (index % MAX_RESOURCES > serverLocks->size()) {
                        // index is in my range but lock doesn't exist
                        ss << ERROR;
                        strcpy(data, ss.str().c_str());
                        outMailHdr.to = incomingMailbox;
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr, outMailHdr, data);
                        break;
                    }
                    printf("Releasing lock %d from machine [%d:%d]\n", index, incomingMachineID, incomingMailbox);
                    success = ReleaseLock(index % MAX_RESOURCES, incomingMachineID, incomingMailbox);
                    ss << SUCCESS;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                }
                break;
            case S_CREATE_CV:
            	ss >> name;
                ss >> requestID;
                ss >> machineID;
                ss >> mailbox;
                ss.clear();
                ss.str("");
                index = FindCV(name);
                if(index == -1) {
                    if (haveSameCreate(name) && (myMachineID < incomingMachineID)) {
                        printf("Have same pending request for CV %s, sending YES to server %d because my machine ID is lower\n", name, incomingMachineID);
                        ss << S_RESPONSE << " " << YES << " " << requestID;
                    } else {
                        printf("Did not find CV %s, sending NO to server %d\n", name, incomingMachineID);
                        ss << S_RESPONSE << " " << NO << " " << requestID;
                    } 
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.from = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);   
                } else {
                    printf("Found CV %s, messaging client [%d:%d] and sending YES to server %d\n", name, machineID, mailbox, incomingMachineID);
                    // Send reply to client with CV index
                    outPktHdr.to = machineID;
                    outMailHdr.to = mailbox;
                    ss << SUCCESS << " " << index;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);

                    ss.clear();
                    ss.str("");

                    //Send yes to other server
                    ss << S_RESPONSE << " " << YES << " " << requestID;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.from = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                }
            	break;
            case CREATE_CV:
                ss >> name;
                ss.clear();
                ss.str("");
                index = FindCV(name);
                if(index == -1) {
                    printf("Did not find CV %s\n", name);
                    requestID = requestCounter;
                    requestCounter++;
                    Request * r = new Request();
                    r->rpcType = rpc;
                    r->name = name;
                    r->noCounter = 0;
                    r->fromMachineID = incomingMachineID;
                    r->fromMailbox = incomingMailbox;
                    requestTable[requestID] = r;
                    //You don't have CV
                    //Ask other servers
                    for (int i = 0; i < NUM_SERVERS; i++)
                    {
                        if(i == myMachineID) {
                            continue;
                        }
                        printf("Sending message to Server %d to check CREATE_CV\n", i);
                        outPktHdr.to = i;
                        outPktHdr.from = myMachineID;
                        outMailHdr.to = 0;
                        outMailHdr.from = 0;
                        ss << S_CREATE_CV << " " << name << " " << requestID << " " << incomingMachineID << " " << incomingMailbox;
                        strcpy(data, ss.str().c_str());
                        ss.clear();
                        ss.str("");
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr,outMailHdr, data);
                    }
                } else {
                    outPktHdr.to = incomingMachineID;
                    outMailHdr.to = incomingMailbox;
                    ss << SUCCESS << " " << index;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, data);
                    printf("Retrieved CV named %s from machine %d\n", name, incomingMachineID);
                }
                break;
            case WAIT:
                ss >> conditionIndex;
                ss >> lockIndex;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = incomingMailbox;
                if(conditionIndex < 0 || static_cast<uint64_t>(conditionIndex) > (MAX_RESOURCES * NUM_SERVERS) - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }

                // See if I have the lcok and then send request to servers if I don't
                if(index < (myMachineID * MAX_RESOURCES) || index > ((myMachineID + 1) * MAX_RESOURCES) - 1) {
                    printf("Did not find lock %d\n", index);
                    requestID = requestCounter;
                    requestCounter++;
                    Request * r = new Request();
                    r->rpcType = rpc;
                    r->noCounter = 0;
                    r->fromMachineID = incomingMachineID;
                    r->fromMailbox = incomingMailbox;
                    requestTable[requestID] = r;
                    //You don't have lock
                    //Ask other servers
                    for (int i = 0; i < NUM_SERVERS; i++)
                    {
                        if(i == myMachineID) {
                            continue;
                        }
                        printf("Sending message to Server %d to check ACQUIRE\n", i);
                        outPktHdr.to = i;
                        outMailHdr.to = 0;
                        // send my (server) address so we will get the lock, not the client
                        ss << S_ACQUIRE << " " << index << " " << requestID << " " << myMachineID << " " << 0;
                        strcpy(data, ss.str().c_str());
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr,outMailHdr, data);
                    }
                } else { // I should have it (it's in my range)
                    if (index % MAX_RESOURCES > serverLocks->size()) {
                        // in my range but invalid
                        ss << ERROR;
                        strcpy(data, ss.str().c_str());
                        outMailHdr.to = incomingMailbox;
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr, outMailHdr, data);
                        break;
                    }
                    success = AcquireLock(index, incomingMachineID, incomingMailbox);
                    if(success) {
                        //Send response to incomingMachineID
                        // do Release (pass parameter to not send msg to requester)
                        // TODO: add extra parameter for ^^ and update existing RELEASE calls
                    } else { // lock is busy, so don't send response
                        printf("Lock %d is busy so machine %d will wait\n", index, incomingMachineID);
                    }
                }                

                // TODO: add switch case for SUCCESS - when server tell's "client" (us as server in this case) lock is acquired
                // SUCCESS case would handle the rest of the CV Wait - code below vvv I believe. But we need to make sure we keep the address straight
                // old code from proj 3
                success = Wait(conditionIndex, lockIndex, incomingMachineID);
                if (success){
                     printf("Machine %d is waiting on lock %d with condition %d\n", incomingMachineID, lockIndex, conditionIndex);
                } else {
                    ss << SUCCESS;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                }
                break;
            case SIGNAL:
                ss >> conditionIndex;
                ss >> lockIndex;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = incomingMailbox;

                if(conditionIndex < 0 || static_cast<uint64_t>(conditionIndex) > (MAX_RESOURCES * NUM_SERVERS) - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }
                Signal(conditionIndex, lockIndex);

                ss << SUCCESS;
                strcpy(data, ss.str().c_str());
                outMailHdr.length = strlen(data) + 1;
                postOffice->Send(outPktHdr, outMailHdr, data);
                break;
            case BROADCAST:
                ss >> conditionIndex;
                ss >> lockIndex;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = incomingMailbox;
                if(conditionIndex < 0 || static_cast<uint64_t>(conditionIndex) > (MAX_RESOURCES * NUM_SERVERS) - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }

                Broadcast(conditionIndex, lockIndex);
                ss << SUCCESS;
                strcpy(data, ss.str().c_str());
                outMailHdr.length = strlen(data) + 1;
                postOffice->Send(outPktHdr, outMailHdr, data);
                break;
            case S_CREATE_MV:
            	ss >> name;
                ss >> requestID;
                ss >> machineID;
                ss >> mailbox;
                ss.clear();
                ss.str("");
                index = FindMV(name);
                if(index == -1) {
                    if (haveSameCreate(name) && (myMachineID < incomingMachineID)) {
                        printf("Have same pending request for MV %s, sending YES to server %d because my machine ID is lower\n", name, incomingMachineID);
                        ss << S_RESPONSE << " " << YES << " " << requestID;
                    } else {
                        printf("Did not find MV %s, sending NO to server %d\n", name, incomingMachineID);
                        ss << S_RESPONSE << " " << NO << " " << requestID;
                    } 
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                } else {
                    printf("Found MV %s, messaging client [%d:%d] and sending YES to server %d\n", name, machineID, mailbox, incomingMachineID);
                    // Send reply to client with CV index
                    outPktHdr.to = machineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = mailbox;
                    outMailHdr.from = 0;
                    ss << SUCCESS << " " << index;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);

                    ss.clear();
                    ss.str("");

                    //Send yes to other server
                    ss << S_RESPONSE << " " << YES << " " << requestID;
                    strcpy(data, ss.str().c_str());
                    outPktHdr.to = incomingMachineID;
                    outPktHdr.from = myMachineID;
                    outMailHdr.to = 0;
                    outMailHdr.from = 0;
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                }
            	break;
            case CREATE_MV:
                ss >> name;
                ss.clear();
                ss.str("");
                index = FindMV(name);
                if(index == -1) {
                    printf("Did not find MV %s\n", name);
                    requestID = requestCounter;
                    requestCounter++;
                    Request * r = new Request();
                    r->rpcType = rpc;
                    r->name = name;
                    r->noCounter = 0;
                    r->fromMachineID = incomingMachineID;
                    r->fromMailbox = incomingMailbox;
                    requestTable[requestID] = r;
                    //You don't have CV
                    //Ask other servers
                    for (int i = 0; i < NUM_SERVERS; i++)
                    {
                        if(i == myMachineID) {
                            continue;
                        }
                        printf("Sending message to Server %d to check CREATE_MV\n", i);
                        outPktHdr.to = i;
                        outPktHdr.from = myMachineID;
                        outMailHdr.to = 0;
                        outMailHdr.from = 0;
                        ss << S_CREATE_MV << " " << name << " " << requestID << " " << incomingMachineID << " " << incomingMailbox;
                        strcpy(data, ss.str().c_str());
                        ss.clear();
                        ss.str("");
                        outMailHdr.length = strlen(data) + 1;
                        postOffice->Send(outPktHdr,outMailHdr, data);
                    }
                } else {
                    outPktHdr.to = incomingMachineID;
                    outMailHdr.to = incomingMailbox;
                    ss << SUCCESS << " " << index;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    success = postOffice->Send(outPktHdr, outMailHdr, data);
                    printf("Retrieved CV named %s from machine %d\n", name, incomingMachineID);
                }
                break;
            case SET_MV:
                ss >> index;
                ss >> mvIndex;
                ss >> value;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = incomingMailbox;
                if(index < 0 || static_cast<uint64_t>(index) > (MAX_RESOURCES * NUM_SERVERS) - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }
                SetMV(index, mvIndex, value);
                ss << SUCCESS;
                strcpy(data, ss.str().c_str());
                postOffice->Send(outPktHdr, outMailHdr, data);
                printf("MV %d has been set to %d\n", index, value);
                break;
            case GET_MV:
                ss >> index;
                ss >> mvIndex;
                ss.clear();
                ss.str("");
                outPktHdr.to = incomingMachineID;
                outMailHdr.to = incomingMailbox;
                if(index < 0 || static_cast<uint64_t>(index) > (MAX_RESOURCES * NUM_SERVERS) - 1) {
                    ss << ERROR;
                    strcpy(data, ss.str().c_str());
                    outMailHdr.length = strlen(data) + 1;
                    postOffice->Send(outPktHdr, outMailHdr, data);
                    break;
                }
                value = GetMV(index, mvIndex); 
                ss << SUCCESS << " " << value;
                strcpy(data, ss.str().c_str());
                postOffice->Send(outPktHdr, outMailHdr, data);
                printf("MV %d has been retrieved with value %d\n", index, value);
                break;
        }
    }
}

bool haveSameCreate(char * name) {
    // check to see if this create resource is in my pending request table
    for (map<int,Request*>::iterator it=requestTable.begin(); it!=requestTable.end(); ++it) {
        if (!strcmp(it->second->name, name)) { // I have a pending request for the same resource
            return true;
        }
    }
    return false;
}

void TakeAction(int requestID) {
    Request * request = requestTable[requestID];
    int rpc = request->rpcType;


    int index;
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    char data[MaxMailSize];
    stringstream ss;
    bool success = false;
    switch(rpc) {
        case CREATE_LOCK:
        	success = true;
            index = CreateLock(request->name);
            break;
        case CREATE_CV:
        	success = true;
        	index = CreateCV(request->name);
            break;
        case CREATE_MV:
        	success = true;
        	index = CreateMV(request->name);
            break;
        default:
            //all other syscalls should have error messages indicating resource is invalid
        	index = -1;
            break;
    }
    outPktHdr.to = request->fromMachineID;
    outMailHdr.to = request->fromMailbox;
    if (success) {
    	ss << SUCCESS << " " << index;
    } else {
    	ss << ERROR << " " << index;
    }
    strcpy(data, ss.str().c_str());
    outMailHdr.length = strlen(data) + 1;
    printf("Created Lock %s at index %d from client [%d:%d]\n", request->name, index, request->fromMachineID, request->fromMailbox);
    postOffice->Send(outPktHdr, outMailHdr, data);
}

int FindLock(char* name) {
    for(unsigned int i = 0; i < serverLocks->size(); i++) {
        ServerLock * l = serverLocks->at(i);
        if (strcmp(l->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int FindCV(char* name) {
    for(unsigned int i = 0; i < serverCVs->size(); i++) {
        ServerCV * l = serverCVs->at(i);
        if (strcmp(l->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int FindMV(char* name) {
    for(unsigned int i = 0; i < serverMVs->size(); i++) {
        ServerMV * l = serverMVs->at(i);
        if (strcmp(l->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int CreateLock(char* name) {    
    ServerLock * newLock = new ServerLock(name);
    int index = lockCounter++;
    serverLocks->push_back(newLock);
    return index;
}

bool AcquireLock(int index, int machineID, int mailbox) {
    ServerLock * sl = serverLocks->at(index);
    bool returnValue = true;
    if(sl->owner.first == -1) {
        sl->owner = make_pair(machineID, mailbox);
        sl->isBusy = true;
    } else if(sl->owner.first == machineID && sl->owner.second == mailbox) {
        //Do nothing I already have the lock
    } else if (sl->isBusy) {
        sl->waitQueue->push_back(make_pair(machineID, mailbox));
        returnValue = false;
    }

    return returnValue;
}

bool ReleaseLock(int index, int machineID, int mailbox) {
    ServerLock * sl = serverLocks->at(index);
    if(sl->owner.first == machineID && sl->owner.second == mailbox) {
        if(sl->waitQueue->empty()) {
            sl->isBusy = false;
            sl->owner = make_pair(-1, 0);
        } else {
            pair<int, int> newOwner = sl->waitQueue->front();
            sl->waitQueue->pop_front();
            sl->owner = newOwner;
            stringstream ss;
            PacketHeader outPktHdr;
            MailHeader outMailHdr;

            outPktHdr.to = newOwner.first;
            outMailHdr.to = newOwner.second;
            ss << SUCCESS;
            char data[MaxMailSize];
            strcpy(data, ss.str().c_str());
            outMailHdr.length = strlen(data) + 1;
            postOffice->Send(outPktHdr, outMailHdr, data);
            printf("Machine [%d:%d] is now the owner of lock %d\n", newOwner.first, newOwner.second, index);
        }
    } else {
        printf("Machine [%d:%d] did not own lock %d so nothing happens\n", machineID, mailbox, index);
    }
    return true;
}

int CreateCV(char* name) {
    ServerCV * newCV = new ServerCV(name);
    int index = cvCounter++;
    serverCVs->push_back(newCV);
    return index;
}

bool Wait(int conditionIndex, int lockIndex, int machineID) {
    ServerCV * cv = serverCVs->at(conditionIndex);
    if(lockIndex < 0 || static_cast<uint64_t>(lockIndex) > serverLocks->size() - 1) {
        return false;
    }
    if (cv->waitingLock == -1) 
    {
        cv->waitingLock = lockIndex;
    }
    if(cv->waitingLock != lockIndex) {
        return false;
    }
    cv->cvWaitQueue->push_back(machineID);
    ReleaseLock(lockIndex, machineID, 0);
    return true;
}

void Signal(int conditionIndex, int lockIndex) {
    ServerCV * cv = serverCVs->at(conditionIndex);
    if(cv->waitingLock == -1) {
        return;
    }
    if(cv->waitingLock != conditionIndex) {
        return;
    }
    if(cv->cvWaitQueue->empty()) {
        cv->waitingLock = -1;
    } else {
        int machineID = cv->cvWaitQueue->front();
        cv->cvWaitQueue->pop_front();
        stringstream ss;
        PacketHeader outPktHdr;
        MailHeader outMailHdr;

        outPktHdr.to = machineID;
        outMailHdr.to = 0;
        ss << SUCCESS;
        char data[MaxMailSize];
        strcpy(data, ss.str().c_str());
        outMailHdr.length = strlen(data) + 1;
        bool success = AcquireLock(lockIndex, machineID, 0);
        printf("Machine %d has been signalled by CV %d\n", machineID, conditionIndex);
        if(success) {
            postOffice->Send(outPktHdr, outMailHdr, data);
        } else {//else lock is busy, so don't send response
            printf("Lock %d is busy so machine %d will wait\n", lockIndex, machineID);
        }
        
    }
}

void Broadcast(int conditionIndex, int lockIndex) {
    ServerCV * cv = serverCVs->at(conditionIndex);
    if(cv->waitingLock != conditionIndex) {
        return;
    }

    while(!cv->cvWaitQueue->empty()) {
        Signal(conditionIndex, lockIndex);
    }
}

int CreateMV(char * name) {
    ServerMV * mv = new ServerMV(name);
    int index = mvCounter++;
    serverMVs->push_back(mv);
    return index;
}

void SetMV(int serverIndex, int mvIndex, int value) {
    ServerMV * mv = serverMVs->at(serverIndex);
    mv->value[mvIndex] = value;
}

int GetMV(int serverIndex, int mvIndex) {
    return serverMVs->at(serverIndex)->value[mvIndex];
}

