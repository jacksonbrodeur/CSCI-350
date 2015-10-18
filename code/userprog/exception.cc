// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "machine.h"
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h> 

using namespace std;

int currentThreadIndex = -1;

int findCurrentProcess() {
    
    int processIndex = -1;
    //we need to iterate through the process table to find the process that this new thread is being forked from
    for(int i = 0; i < 100; i ++) {
        
        //if this evaluates to true, we have found the current process
        if(processTable[i]!=NULL && processTable[i]->mySpace == currentThread->space) {
            
            processIndex = i;
            break;
        }
    }
    return processIndex;
}

int validateLock(int index) {
    if (index < 0 || index > MAX_LOCKS) {
        printf("The index: %d was invalid.\n", index);
        return 0;
    }
    if (kernelLocks[index]->lock == NULL) {
        printf("There is no lock at index: %d\n", index);
        return 0;
    }
    if (kernelLocks[index]->addrSpace != currentThread->space)
    {
        printf("The lock belongs to a different process\n");
        return 0;
    }
    return 1;
}

int validateCV(int index) {
    if (index < 0 || index > MAX_LOCKS) {
        printf("The index: %d was invalid.\n", index);
        return 0;
    }
    if (kernelCVs[index]->condition == NULL) {
        printf("There is no condition at index: %d\n", index);
        return 0;
    }
    if (kernelCVs[index]->addrSpace != currentThread->space)
    {
        printf("The CV belongs to a different process\n");
        return 0;
    }
    return 1;
}

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	   //translation failed
	   return -1;
    }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
    	printf("%s","Bad pointer passed to Create\n");
    	delete buf;
    	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
    	printf("%s","Can't allocate kernel buffer in Open\n");
    	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

void exec_thread(int vaddr) {
    
    execLock->Acquire();


    int ppn = stackBitMap->Find();
    
    if(ppn==-1) {
        
        printf("Bitmap Find returned -1, halting\n");
        interrupt->Halt();
    }
    
    int startingStackPage = PageSize * (currentThread->space->codeDataPages + (ppn + 1) * 8) - 16;
    
    
    processTable[findCurrentProcess()]->threadList[0]->startingStackPage = startingStackPage;
    currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table register
    
    execLock->Release();
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
}

int ExecSyscall(int vaddr, int len) {
    
    execLock->Acquire();
    
    char * filename = new char[len+1];
    AddrSpace * mySpace;

    if(!filename) {
        
        printf("Error allocating kernel file name buffer in exec\n");
        return -1;
    }
    
    if ( copyin(vaddr, len, filename) == -1 ) {
        printf("%s","Bad pointer passed to copyin\n");
    }
    
    
    filename[len]='\0';
                    
    
    OpenFile *executable = fileSystem->Open(filename);
    
    delete[] filename;
    
    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return -1;
    }
    
    mySpace = new AddrSpace(executable);
    
    Thread * t = new Thread("executable thread");
    
    t->space = mySpace;
    
    delete executable;			// close file
    
    KernelProcess * newProcess = new KernelProcess(t);
    newProcess->mySpace = mySpace;
    
    int index = -1;

    for(int i =0;i < 100;i ++) {
        
        if(processTable[i]==NULL) {
            processTable[i] = newProcess;
            index = i;
            break;
        }
    }
    
    KernelThread * newThread = new KernelThread(t);
    newProcess->threadList[0]=newThread;
    
    execLock->Release();
    t->Fork(exec_thread, vaddr);
    return index;
}

bool isLastExecutingThread() {
    
    bool isLastThread = true;
    
    for(int i = 0; i < 100 ; i++) {
        
        if(processTable[i]!=NULL) {
            
            for(int j = 0; j < 50; j++)
                if(processTable[i]->threadList[j]!=NULL && (processTable[i]->threadList[j]->myThread)!=currentThread) {
                
                    isLastThread = false;
                }
        }
    }
    return isLastThread;
}

bool isLastExecutingProcess() {
    
    //here we can just check our system.h/.cc process table to see if this is the only thread left in it
    
    for(int i = 0; i <100;i++) {
        
        if(processTable[i]!=NULL && processTable[i]->mySpace!=currentThread->space) {
            
            return false;
        }
    }
    
    return true;
}

void ExitSyscall(int status) {

    exitLock->Acquire();
    
    int currentProcess = findCurrentProcess();
    //check if this thread is the last process
    if(!isLastExecutingThread()) {
        //reclaim 8 pages of stack (keep track of where currentthreads stack pages are)
        int threadListIndex = -1;
        //iterate through the current process's thread list to find the exiting thread
        for(int i = 0; i < 50; i ++) {
            
            if(processTable[currentProcess]->threadList[i]->myThread == currentThread) {
                //keep track of the exiting thread's index in the process's thread list
                threadListIndex = i;
                break;
            }
        }
        //reclaim the exiting threads memory
        stackBitMap->Clear(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage);
        //one thread has finished executing so keep track of this in the current process
        processTable[currentProcess]->numThreadsExecuting--;
    }
    else if(isLastExecutingThread() && isLastExecutingProcess()) {
        interrupt->Halt();
    }
    else if(isLastExecutingThread() && !isLastExecutingProcess()) {
        
        //reclaim 8 pages of stack (keep track of where currentthreads stack pages are)
        int threadListIndex = -1;
        //iterate through the current process's thread list to find the exiting thread
        for(int i = 0; i < 50; i ++) {
            
            if(processTable[currentProcess]->threadList[i]->myThread == currentThread) {
                //keep track of the exiting thread's index in the process's thread list
                threadListIndex = i;
                break;
            }
        }

        //reclaim the exiting threads memory
        stackBitMap->Clear(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage);
        
        for(int i =0;i<MAX_LOCKS;i++) {
            if(kernelLocks[i]->addrSpace == currentThread->space) {
                
                kernelLocks[i]->lock=NULL;
            }
            if(kernelCVs[i]->addrSpace == currentThread->space) {
                
                kernelCVs[i]->condition=NULL;
            }
        }
        
        for(int i = 0; i < 100; i ++) {
            
            if(processTable[i]!=NULL) {
                if(processTable[i]->mySpace == currentThread->space) {
        
                    processTable[i] = NULL;
                }
            }
        }
        
    }
    
    exitLock->Release();
    currentThread->Finish();
}

int findAvailableThreadListIndex (int processIndex) {
    
    int threadListIndex = -1;
    
    //now iterate through the thread list of this process to find an empty location
    for(int j = 0; j < 50; j ++) {
        
        //we have found an empty spot in the list, now we store our new KernelThread there
        if(processTable[processIndex]->threadList[j]==NULL) {
            
            threadListIndex = j;
            break;
        }
    }
    
    //printf("Found a new threadListIndex, %d\n", threadListIndex);
    
    return threadListIndex;
}

void kernel_thread(int vaddr) {
    
    //printf("Vaddr: %d\n", vaddr);
    forkLock->Acquire();
    //IntStatus old = interrupt->SetLevel(IntOff);
    
    //printf("We are inside the kernel_thread method\n");
    
    currentThread->space->InitRegisters();
    
    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr + 4);
    
    int ppn = stackBitMap->Find();
    
    if(ppn==-1) {
        
        printf("Bitmap Find returned -1, halting\n");
        interrupt->Halt();
    }
    
    int startingStackPage = PageSize * (currentThread->space->codeDataPages + (ppn + 1) * 8) - 16;
    
    //printf("The starting stack page is %d\n",startingStackPage);
    
    //find the current process and set the new threads starting stack page variable
    int currentProcess = findCurrentProcess();
    //printf("The current process is %d and the current thread index in that process is %d\n", currentProcess, currentThreadIndex);
    
    for(int i = 0; i < 50; i++) {
        
        if(processTable[currentProcess]->threadList[i]->myThread == currentThread)
        {
            currentThreadIndex = i;
            break;
        }
    }
    
    //printf("The thread that is about to be forked will have index %d in the process\n", currentThreadIndex);
    
    processTable[currentProcess]->threadList[currentThreadIndex]->startingStackPage = startingStackPage;
    
    //printf("Assigning process %d thread %d starting stack page value of %d\n", currentProcess,currentThreadIndex,startingStackPage);
    
    machine->WriteRegister(StackReg, startingStackPage);
    
    currentThread->space->RestoreState();
    
    //(void) interrupt->SetLevel(old);
    forkLock->Release();
    machine->Run();
}

void ForkSyscall(int vaddr) {

    forkLock->Acquire();
    //IntStatus old = interrupt->SetLevel(IntOff);
    
    //printf("Entering fork syscall\n");
    
    Thread * t = new Thread("forking thread");
    
    t->space = currentThread->space;
    
    KernelThread * newThread = new KernelThread(t);
    
    //find the current process running (the process this new thread being forked belongs to) and set the global variable to keep track of where this thread is stored in the current process's thread list
    int currentProcess = findCurrentProcess();
    
    //printf("The current process running is proccess %d\n", currentProcess);
    
    currentThreadIndex = findAvailableThreadListIndex(currentProcess);
    
    //printf("The thread that is about to be forked will have index %d in the process\n", currentThreadIndex);
    
    processTable[currentProcess]->threadList[currentThreadIndex] = newThread;
    
    processTable[currentProcess]->totalThreads++;
    processTable[currentProcess]->numThreadsExecuting++;
    
    //printf("We are forking the new thread with vaddr: %d\n", vaddr);
    
    forkLock->Release();
    t->Fork(kernel_thread, vaddr);
    //(void) interrupt->SetLevel(old);
}

void YieldSyscall() {

    printf("Current thread: %s\n", currentThread->getName());
    currentThread->Yield();
    printf("Current thread: %s\n", currentThread->getName());
}

// Returns -1 if there is an error
int CreateLockSyscall(int vaddr, int len) {
  
    char * name = new char[len+1];

    if(len < 0 || len > MAXFILENAME) {
    printf("Invalid string length in CreateLockSyscall\n");
    return -1;
    }

    if(copyin(vaddr, len, name) == -1) {
    printf("Bad vaddr passed in to CreateLockSyscall\n");
    return -1;
    }

    name[len] = '\0';

    KernelLock * kernelLock = new KernelLock(name);

    lockTableLock->Acquire();
    int index = -1;
    for(int i = 0; i < MAX_LOCKS; i++) {
      if (kernelLocks[i]->lock == NULL) {
        kernelLocks[i] = kernelLock;
        index = i;
        break;
        }
    }

    //DEBUG('d', "Creating Lock: %s\n", kernelLocks[index]->lock->getName());
    printf("Creating Lock: %s\n", kernelLocks[index]->lock->getName());

    lockTableLock->Release();
    return index;
}

void DestroyLockSyscall(int index) {
    lockTableLock->Acquire();
    if(validateLock(index)) {
        if(!kernelLocks[index]->lock->isInUse()) {
            //see if the lock is busy, delete it here immediately if it is not
            kernelLocks[index]->lock = NULL;
            printf("Deleting lock %d now\n",index);
        } else {
            //otherwise mark the lock for deletion and wait for people to stop using it
            kernelLocks[index]->isToBeDeleted = true;
            printf("Marking lock %d for deletion\n", index);
        }
    }
    lockTableLock->Release();
}

int AcquireSyscall(int index) {

    lockTableLock->Acquire();
    if(validateLock(index)) {
        
        if(kernelLocks[index]->lock->isInUse())
            printf("Lock is busy so I will wait\n");
        else
            printf("Lock is available so I will be the owner\n");
        
        kernelLocks[index]->lock->Acquire();

        lockTableLock->Release();
        
        return 1;
    }
    
    lockTableLock->Release();
    printf("Lock is invalid\n");
    return 0;
}

void ReleaseSyscall(int index) {

    lockTableLock->Acquire();
    if(validateLock(index)) {
        kernelLocks[index]->lock->Release();
    
        // if the lock is not in use (owner == NULL) then delete it (set lock to NULL)
        if(kernelLocks[index]->isToBeDeleted && !(kernelLocks[index]->lock->isInUse())) {
            kernelLocks[index]->lock = NULL;
        }
    }
    lockTableLock->Release();
}

int CreateConditionSyscall(int vaddr, int len) {
  
    cvTableLock->Acquire();
    
    char * name = new char[len+1];
    
    if(len < 0 || len > MAXFILENAME) {
        printf("Invalid string length in CreateConditionSyscall\n");
        cvTableLock->Release();
        return -1;
    }
    
    if(copyin(vaddr, len, name) == -1) {
        printf("Bad vaddr passed in to CreateLockSyscall\n");
        cvTableLock->Release();
        return -1;
    }
    
    name[len] = '\0';
    
    //Create the condition variable here
    KernelCV * kernelCV = new KernelCV(name);

    int index = -1;
    for(int i = 0; i < MAX_LOCKS; i++) {
        if (kernelCVs[i]->condition == NULL) {
            kernelCVs[i] = kernelCV;
            index = i;
            break;
        }
    }
    
    printf("Creating Condition: %s\n", kernelCVs[index]->condition->getName());
    DEBUG('d', "Creating Condition: %s\n", kernelCVs[index]->condition->getName());
    cvTableLock->Release();
    return index;
}

void DestroyConditionSyscall(int index) {
    cvTableLock->Acquire();
    if(validateCV(index)) {
        if(!kernelCVs[index]->condition->isInUse()) {
            //see if the CV is busy, delete it here immediately if it is not
            kernelCVs[index]->condition = NULL;
        } else {
            // otherwise mark the CV for deletion and wait for people to stop using it
            kernelCVs[index]->isToBeDeleted = true;
        }
    }    
    cvTableLock->Release();
}


void WaitSyscall(int conditionIndex, int lockIndex) {
    cvTableLock->Acquire();
    if(validateCV(conditionIndex) && validateLock(lockIndex)) {
        DEBUG('d', "Waiting on condition %s with lock %s\n", kernelCVs[conditionIndex]->condition->getName(),
            kernelLocks[lockIndex]->lock->getName());
        cvTableLock->Release();
        kernelCVs[conditionIndex]->condition->Wait(kernelLocks[lockIndex]->lock);
    }
    cvTableLock->Release();
}

void SignalSyscall(int conditionIndex, int lockIndex) {
    cvTableLock->Acquire();
    if(validateCV(conditionIndex) && validateLock(lockIndex)) {
        DEBUG('d', "Signalling on condition %s with lock %s\n", kernelCVs[conditionIndex]->condition->getName(),
            kernelLocks[lockIndex]->lock->getName());
        kernelCVs[conditionIndex]->condition->Signal(kernelLocks[lockIndex]->lock);
    } else {
        DEBUG('d', "Invalid lock or condition to Signal\n");
    }
    cvTableLock->Release();
}

void BroadcastSyscall(int conditionIndex, int lockIndex) {
    cvTableLock->Acquire();
    if(validateCV(conditionIndex) && validateLock(lockIndex)) {
        DEBUG('d', "Broadcasting on condition %s with lock %s\n", kernelCVs[conditionIndex]->condition->getName(),
            kernelLocks[lockIndex]->lock->getName());
        kernelCVs[conditionIndex]->condition->Broadcast(kernelLocks[lockIndex]->lock);
    }
    cvTableLock->Release();
}

void PrintSyscall(int vaddr, int len, int params1, int params2) {

  if(len < 0 || len > MAXFILENAME) {
        printf("Invalid string length in CreateLockSyscall\n");
        return;
  }

  char * string = new char[len + 1];

  if(copyin(vaddr, len, string) == -1) {
        printf("Bad vaddr passed in to CreateLockSyscall\n");
        return;
  }
    
  int * params = new int[4];
  params[0] = params1 / 1000;
  params[1] = params1 % 1000;
  params[2] = params2 / 1000;
  params[3] = params2 % 1000;
  int index = 0;
  string[len] = '\0';

  printLock->Acquire();
  //Prevent another print syscall from being executed until this one is done
  for(int i = 0; i < len; i++) {
    if (string[i] == '%')
    {
      if(string[i+1] == 'i'){
        printf("%i", params[index]);
        index++;
        i++;
      }
    } else if(string[i] == '\\') {
        if(string[i+1] == 'n') {
            printf("\n");
        }
    } else {
      printf("%c", string[i]);
    }
  }
  printLock->Release();

}

int RandSyscall() {
    return rand();
}


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
        switch (type) {
            default:
                DEBUG('a', "Unknown syscall - shutting down.\n");
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Create:
                DEBUG('a', "Create syscall.\n");
                Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Open:
                DEBUG('a', "Open syscall.\n");
                rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Write:
                DEBUG('a', "Write syscall.\n");
                Write_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5),
                              machine->ReadRegister(6));
                break;
            case SC_Read:
                DEBUG('a', "Read syscall.\n");
                rv = Read_Syscall(machine->ReadRegister(4),
                                  machine->ReadRegister(5),
                                  machine->ReadRegister(6));
                break;
            case SC_Close:
                DEBUG('a', "Close syscall.\n");
                Close_Syscall(machine->ReadRegister(4));
                break;
                
            case SC_Exec:
                DEBUG('a', "Exec syscall.\n");
                rv = ExecSyscall(machine->ReadRegister(4),
                                 machine->ReadRegister(5));
                break;
                
            case SC_Exit:
                DEBUG('a', "Exit syscall.\n");
                ExitSyscall(machine->ReadRegister(4));
                break;
                
            case SC_Fork:
                DEBUG('a', "Fork syscall.\n");
                ForkSyscall(machine->ReadRegister(4));
                break;
                
            case SC_Yield:
                DEBUG('a', "Yield syscall.\n");
                YieldSyscall();
                break;
                
            case SC_CreateLock:
                DEBUG('a', "CreateLock syscall.\n");
                rv = CreateLockSyscall(machine->ReadRegister(4),
                                       machine->ReadRegister(5));
                break;
                
            case SC_DestroyLock:
                DEBUG('a', "DestroyLock syscall.\n");
                DestroyLockSyscall(machine->ReadRegister(4));
                break;
                
            case SC_Acquire:
                DEBUG('a', "Acquire syscall.\n");
                AcquireSyscall(machine->ReadRegister(4));
                break;
                
            case SC_Release:
                DEBUG('a', "Release syscall.\n");
                ReleaseSyscall(machine->ReadRegister(4));
                break;
                
            case SC_CreateCondition:
                DEBUG('a', "CreateCondition syscall.\n");
                rv = CreateConditionSyscall(machine->ReadRegister(4),
                                            machine->ReadRegister(5));
                break;
                
            case SC_DestroyCondition:
                DEBUG('a', "DestroyCondition syscall.\n");
                DestroyConditionSyscall(machine->ReadRegister(4));
                break;
                
            case SC_Wait:
                DEBUG('a', "Wait syscall.\n");
                WaitSyscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
                
            case SC_Signal:
                DEBUG('a', "Signal syscall.\n");
                SignalSyscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
                
            case SC_Broadcast:
                DEBUG('a', "Broadcast syscall.\n");
                BroadcastSyscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
                
            case SC_Print:
                DEBUG('a', "Print syscall.\n");
                PrintSyscall(machine->ReadRegister(4), 
                             machine->ReadRegister(5), 
                             machine->ReadRegister(6), 
                             machine->ReadRegister(7));
                break;

            case SC_Rand:
                DEBUG('a', "Rand syscall.\n");
                rv = RandSyscall();
                break;
        }
        
        // Put in the return value and increment the PC
        machine->WriteRegister(2,rv);
        machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
        return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
