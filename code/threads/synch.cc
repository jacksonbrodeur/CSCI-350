// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include "stddef.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

Lock::Lock() {
    
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {

    name = debugName;
    waitQueue = new List;
    isBusy = false;
    owner = NULL;
}

Lock::~Lock() {

    delete waitQueue;
}

void Lock::Acquire() {

    IntStatus old = interrupt->SetLevel(IntOff);
    
    if (isHeldByCurrentThread()) {
        printf("This lock is held by %s already \n", currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    if(owner == NULL) {
        printf("The lock is available so %s is now the owner \n", currentThread->getName());
        isBusy = true;
        owner = currentThread;
    } else {
        printf("This lock is held by another thread already so we are adding %s to the waitQueue \n", currentThread->getName());
        //lock is busy
        waitQueue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    
    (void) interrupt->SetLevel(old);
}

void Lock::Release() {

    IntStatus old = interrupt->SetLevel(IntOff);
    if (!isHeldByCurrentThread()) {
        printf("ERROR: The current thread is not the lock owner. (%s) \n", currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    if (!waitQueue->IsEmpty()) {

        owner = (Thread *)waitQueue->Remove();
        scheduler->ReadyToRun(owner);
    } else {
        isBusy = false;
        owner = NULL;
    }
    (void) interrupt->SetLevel(old);
}

bool Lock::isHeldByCurrentThread() {
    
    return (currentThread == owner);
}

Condition::Condition() {

}

Condition::Condition(char* debugName) {

    name = debugName;
    cvWaitQueue = new List;
    waitingLock = NULL;
}

Condition::~Condition() {

    delete cvWaitQueue;
}

void Condition::Wait(Lock* conditionLock) {
    
    IntStatus old = interrupt->SetLevel(IntOff);
    if(conditionLock == NULL) {
        
        printf("Error, conditionLock is NULL inside wait (%s) \n", currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    if(waitingLock==NULL) {
        printf("Inside Wait, setting conditionLock equal to waitingLock (%s) \n", currentThread->getName());
        waitingLock = conditionLock;
    }
    if(waitingLock != conditionLock) {
        
        printf("Inside wait: waiting on different lock (%s) \n", currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    cvWaitQueue->Append((void *)currentThread);
    conditionLock->Release();
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(old);
}

void Condition::Signal(Lock* conditionLock) {

    IntStatus old = interrupt->SetLevel(IntOff);
    if(waitingLock == NULL) {
        printf("Inside signal function and there is no waitingLock (%s) \n",currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    if(waitingLock != conditionLock) {
        
        printf("ERROR Inside signal: Different lock than went to sleep with (%s) \n", currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    if(cvWaitQueue->IsEmpty()){
        
        waitingLock = NULL;
    }
    else{
        Thread *waitingThread = (Thread *)cvWaitQueue->Remove();
        scheduler->ReadyToRun(waitingThread);
    }
    (void) interrupt->SetLevel(old);
}

void Condition::Broadcast(Lock* conditionLock) {

    IntStatus old = interrupt->SetLevel(IntOff);
    if(conditionLock==NULL){
        
        printf("ERROR: Lock passed in was null (%s) \n", currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    if(conditionLock!=waitingLock){
        
        printf("ERROR Broadcast: Different lock than went to sleep with (%s) \n", currentThread->getName());
        (void) interrupt->SetLevel(old);
        return;
    }
    (void) interrupt->SetLevel(old);
    
    while(!cvWaitQueue->IsEmpty()) {
        
        Signal(conditionLock);
    }
    
}
