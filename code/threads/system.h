// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"
#include "bitmap.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

struct KernelLock
{
    Lock *lock;
    AddrSpace *addrSpace;
    bool isToBeDeleted;

    KernelLock();

    KernelLock(char * name);
};

struct KernelCV
{
    Condition * condition;
    AddrSpace *addrSpace;
    bool isToBeDeleted;
    
    KernelCV();
    
    KernelCV(char * name);
};

struct KernelThread
{
    int startingStackPage;
    Thread * myThread;
   
    KernelThread();
    
    KernelThread(Thread* userThread);
};

struct KernelProcess
{
    KernelThread ** threadList;
    Thread * myThread;
    int totalThreads;
    int numThreadsExecuting;
    AddrSpace * mySpace;
    BitMap * stackBitMap;
    
    KernelProcess();
    
    KernelProcess(Thread * processThread);
};

struct IPT : public TranslationEntry {
    
    AddrSpace* mySpace;
};


#define MAX_LOCKS 1000
#define TOTALPAGESPERPROCESS 1000

#define EXECUTABLE 0
#define SWAPFILE 1
#define NEITHER 2
#define FIFO 0
#define RAND 1
extern int pageReplacementPolicy;
extern List* pageQueue;
extern KernelLock** kernelLocks;
extern KernelCV** kernelCVs;

extern Lock * lockTableLock;
extern Lock * cvTableLock;
extern Lock * printLock;
extern Lock * processTableLock;

extern Lock * memoryLock;
extern Lock * iptLock;

extern KernelProcess** processTable;
extern BitMap * physicalPageBitMap;
extern BitMap *swapFileBitMap;

extern OpenFile * swapFile;

extern IPT *ipt;

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

//Define codes for sending syscalls to server
#define CREATE_LOCK      0
#define DESTROY_LOCK     1
#define ACQUIRE          2
#define RELEASE          3
#define CREATE_CV        4
#define DESTROY_CV       5
#define WAIT             6
#define SIGNAL           7
#define BROADCAST        8
#define CREATE_MV        9
#define DESTROY_MV      10
#define GET_MV          11
#define SET_MV          12

#define S_RESPONSE      13
#define S_CREATE_LOCK   14
#define S_CREATE_CV     15
#define S_CREATE_MV     16
#define S_ACQUIRE       17
#define S_RELEASE       18


//Define codes for responses from server
#define ERROR           0
#define SUCCESS         1



#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
extern int myMachineID;
#endif

#endif // SYSTEM_H
