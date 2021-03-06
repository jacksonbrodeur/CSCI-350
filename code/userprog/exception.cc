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

#include <sstream>
#include "machine.h"
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h> 

using namespace std;

int currentThreadIndex = -1;

int currentTLB = 0;

int findCurrentProcess() {
    
    int processIndex = -1;
    //we need to iterate through the process table to find the process that this new thread is being forked from
    for(int i = 0; i < 100; i ++) {
        
        //means the process at i exists and has the same space as the current process (only way this can happen is if it IS the current process)
        if(processTable[i]!=NULL && processTable[i]->mySpace == currentThread->space) {
            
            processIndex = i;
            break;
        }
    }
    return processIndex;
}

int validateLock(int index) {
    
    if (index < 0 || index > MAX_LOCKS) {
        printf("The lock index: %d was invalid.\n", index);
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
        printf("The condition index: %d was invalid.\n", index);
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

    
    processTableLock->Acquire();
    DEBUG('b', "inside of exec thread with vaddr of %d\n", vaddr);
    DEBUG('b', "addrspace: %d\n", currentThread->space);

    
    currentThread->space->InitRegisters();		// set the initial register values
    
    //give the main thread of this process a starting stack page
    int ppn = processTable[findCurrentProcess()]->stackBitMap->Find();
    
    if(ppn==-1) {
        
        printf("Bitmap Find returned -1, halting\n");
        interrupt->Halt();
    }
    
    int startingStackPage = PageSize * (currentThread->space->codeDataPages + (ppn + 1) * 8) - 16;
    
    DEBUG('b', "Giving the process %d thread a starting stack page of %d\n", findCurrentProcess(), startingStackPage);
    
    processTable[findCurrentProcess()]->threadList[0]->startingStackPage = startingStackPage;
    
    machine->WriteRegister(StackReg, startingStackPage);

    currentThread->space->RestoreState();		// load page table register
    
    processTableLock->Release();
    
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
}

int ExecSyscall(int vaddr, int len) {
    
    processTableLock->Acquire();
    
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
    }
    
    //make a new address space, a new process, and a new thread to be the main thread of this process
    mySpace = new AddrSpace(executable);
    
    Thread * t = new Thread("executable thread");
    t->myMailbox = mailBoxCounter;
    mailBoxCounter++;
    
    t->space = mySpace;
    
    BitMap * stackBitMap = new BitMap(TOTALPAGESPERPROCESS);
    
    //delete executable;			// close file
    
    KernelProcess * newProcess = new KernelProcess(t);
    newProcess->mySpace = mySpace;
    newProcess->stackBitMap = stackBitMap;
    
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
    
    DEBUG('b', "Created a new process and a new kernel thread with addrspace: %d\n", mySpace);
    DEBUG('b', "vaddr is %d\n",vaddr);
    
    t->Fork(exec_thread, vaddr);
    processTableLock->Release();
    
    return index;
}

bool isLastExecutingThread(int process) {
    
    bool isLastThread = true;

    if(processTable[process]!=NULL) {
            
        for(int i = 0; i < 100; i++)
            if(processTable[process]->threadList[i]!=NULL && (processTable[process]->threadList[i]->myThread)!=currentThread) {
            
                isLastThread = false;
                break;
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
    
    //currentThread->Finish();
    
    processTableLock->Acquire();
    
    printf("Exiting with status %d\n",status);
    
    int currentProcess = findCurrentProcess();
    //check if this thread is the last process
    if(processTable[currentProcess]->numThreadsExecuting > 1) {
        //reclaim 8 pages of stack (keep track of where currentthreads stack pages are)
        int threadListIndex = -1;
        //iterate through the current process's thread list to find the exiting thread
        for(int i = 0; i < 100; i ++) {
            
            if(processTable[currentProcess]->threadList[i]->myThread == currentThread) {
                //keep track of the exiting thread's index in the process's thread list
                threadListIndex = i;
                break;
            }
        }
        //reclaim the exiting threads memory
        for (int i = 0; i < 8; i++) {
            
            //printf("About to clear physical page: %d\n", currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize + i)].physicalPage);
            
            if(currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].valid) {
                
                memoryLock->Acquire();
                
                physicalPageBitMap->Clear(currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].physicalPage);
                
                processTable[currentProcess]->stackBitMap->Clear(currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].virtualPage);
                
                memoryLock->Release();
                
                iptLock->Acquire();
                
                ipt[currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].physicalPage].valid = FALSE;
                
                currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].valid = FALSE;
                
                iptLock->Release();
                
            }
        }
        //one thread has finished executing so keep track of this in the current process
        processTable[currentProcess]->numThreadsExecuting--;
    }
    else if(processTable[currentProcess]->numThreadsExecuting == 1 && isLastExecutingProcess()) {
        printf("System done\n");
        interrupt->Halt();
    }
    else if(processTable[currentProcess]->numThreadsExecuting == 1 && !isLastExecutingProcess()) {
        
        //reclaim 8 pages of stack (keep track of where currentthreads stack pages are)
        int threadListIndex = -1;
        //iterate through the current process's thread list to find the exiting thread
        for(int i = 0; i < 100; i ++) {
            
            if(processTable[currentProcess]->threadList[i]->myThread == currentThread) {
                //keep track of the exiting thread's index in the process's thread list
                threadListIndex = i;
                break;
            }
        }

        //reclaim the exiting threads memory
        for (int i = 0; i < 8; i++) {
            
            if(ipt[currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].physicalPage].valid) {
                
                memoryLock->Acquire();

                
                physicalPageBitMap->Clear(currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].physicalPage);
                
                memoryLock->Release();
                
                iptLock->Acquire();
                ipt[currentThread->space->pageTable[(processTable[currentProcess]->threadList[threadListIndex]->startingStackPage/PageSize - i)].physicalPage].valid = FALSE;
                iptLock->Release();
            }
        }

        for(int i =0;i<MAX_LOCKS;i++) {
            if(kernelLocks[i]->addrSpace == currentThread->space) {
                
                kernelLocks[i]->lock=NULL;
            }
            if(kernelCVs[i]->addrSpace == currentThread->space) {
                
                kernelCVs[i]->condition=NULL;
            }
        }
        
        processTable[currentProcess] = NULL;
        
    }
    
    processTableLock->Release();
    currentThread->Finish();
}

int findAvailableThreadListIndex (int processIndex) {
    
    int threadListIndex = -1;
    
    //now iterate through the thread list of this process to find an empty location
    for(int j = 0; j < 100; j ++) {
        
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
    processTableLock->Acquire();
    
    //printf("We are inside the kernel_thread method\n");
    
    int currentProcess = findCurrentProcess();
    
    currentThread->space->InitRegisters();
    
    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr + 4);
    
    int ppn = processTable[currentProcess]->stackBitMap->Find();
    
    if(ppn==-1) {
        
        printf("Bitmap Find returned -1, halting\n");
        interrupt->Halt();
    }
    
    int startingStackPage = PageSize * (currentThread->space->codeDataPages + (ppn + 1) * 8) - 16;
    
    //printf("The starting stack page is %d\n",startingStackPage);
    
    //find the current process and set the new threads starting stack page variable
    //printf("The current process is %d and the current thread index in that process is %d\n", currentProcess, currentThreadIndex);
    
    for(int i = 0; i < 100; i++) {
        
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
    
    processTableLock->Release();
    machine->Run();
}

void ForkSyscall(int vaddr) {

    processTableLock->Acquire();
    
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
    
    processTableLock->Release();
    t->Fork(kernel_thread, vaddr);
}

void YieldSyscall() {

    currentThread->Yield();
}



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

    PacketHeader pktHdr;
    MailHeader mailHdr;

    //Server always should have machineID=0
    
    stringstream ss;
    ss << CREATE_LOCK << " " << name;
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    int index;
    ss >> code;
    if(code != SUCCESS) {
        printf("CreateLock has failed which should not happen, Terminating Program\n");
        interrupt->Halt();
    }
    ss >> index;
    ASSERT(index >= 0);
    return index;
}

void DestroyLockSyscall(int index) {
    lockTableLock->Acquire();
    if(validateLock(index)) {
        if(!kernelLocks[index]->lock->isInUse()) {
            //see if the lock is busy, delete it here immediately if it is not
            kernelLocks[index]->lock = NULL;
        } else {
            //otherwise mark the lock for deletion and wait for people to stop using it
            kernelLocks[index]->isToBeDeleted = true;
            printf("Marking lock %d for deletion\n", index);
        }
    }
    lockTableLock->Release();
}

int AcquireSyscall(int index) {
    stringstream ss;
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ss << ACQUIRE << " " << index;

    //Server always should have machineID=0
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    ss >> code;
    if(code != SUCCESS) {
        printf("Lock is invalid");
        return ERROR;
    }
    return SUCCESS;
}

void ReleaseSyscall(int index) {

    stringstream ss;
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ss << RELEASE << " " << index;

    //Server always should have machineID=0
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    ss >> code;
    if(code != SUCCESS) {
        printf("Lock is invalid");
    }
}

int CreateConditionSyscall(int vaddr, int len) {
    
    char * name = new char[len+1];
    
    if(len < 0 || len > MAXFILENAME) {
        printf("Invalid string length in CreateConditionSyscall\n");
        cvTableLock->Release();
        return -1;
    }
    
    if(copyin(vaddr, len, name) == -1) {
        printf("Bad vaddr passed in to CreateConditionSyscall\n");
        return -1;
    }
    
    name[len] = '\0';

    PacketHeader pktHdr;
    MailHeader mailHdr;

    //Server always should have machineID=0
    
    stringstream ss;
    ss << CREATE_CV << " " << name;
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    int index;
    ss >> code;
    if(code != SUCCESS) {
        printf("CreateCV has failed which should not happen, Terminating Program\n");
        interrupt->Halt();
    }
    ss >> index;
    ASSERT(index >= 0);
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

    stringstream ss;
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ss << WAIT << " " << conditionIndex << " " << lockIndex;

    //Server always should have machineID=0
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    ss >> code;
    if(code != SUCCESS) {
        printf("Condition is invalid\n");
    }

    AcquireSyscall(lockIndex);
}

void SignalSyscall(int conditionIndex, int lockIndex) {
    stringstream ss;
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ss << SIGNAL << " " << conditionIndex << " " << lockIndex;

    //Server always should have machineID=0
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    ss >> code;
    if(code != SUCCESS) {
        printf("Condition is invalid\n");
    }
}

void BroadcastSyscall(int conditionIndex, int lockIndex) {

    stringstream ss;
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ss << BROADCAST << " " << conditionIndex << " " << lockIndex;

    //Server always should have machineID=0
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    ss >> code;
    if(code != SUCCESS) {
        printf("Condition is invalid");
    }
}

void PrintSyscall(int vaddr, int len, int params1, int params2) {

  printLock->Acquire();
  if(len < 0 || len > MAXFILENAME) {
        printf("Invalid string length in PrintSyscall\n");
        return;
  }

  char * string = new char[len + 1];

  if(copyin(vaddr, len, string) == -1) {
        printf("Bad vaddr passed in to PrintSyscall\n");
        return;
  }
    
  int * params = new int[4];
  params[0] = params1 / 1000;
  params[1] = params1 % 1000;
  params[2] = params2 / 1000;
  params[3] = params2 % 1000;
  int index = 0;
  string[len] = '\0';

  
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
  delete string;
  printLock->Release();

}

int RandSyscall() {
    return rand();
}

int CreateMVSyscall(int vaddr, int len) {
    char * name = new char[len+1];
    
    if(len < 0 || len > MAXFILENAME) {
        printf("Invalid string length in CreateMVSyscall\n");
        return -1;
    }
    
    if(copyin(vaddr, len, name) == -1) {
        printf("Bad vaddr passed in to CreateMVSyscall\n");
        cvTableLock->Release();
        return -1;
    }
    
    name[len] = '\0';

    PacketHeader pktHdr;
    MailHeader mailHdr;

    //Server always should have machineID=0
    
    stringstream ss;
    ss << CREATE_MV  << " " << name;
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    int index;
    ss >> code;
    if(code != SUCCESS) {
        printf("CreateMV has failed which should not happen, Terminating Program\n");
        interrupt->Halt();
    }
    ss >> index;
    ASSERT(index >= 0);
    return index;
}

void DestroyMVSyscall(int index) {

}

void SetSyscall(int index, int mvIndex, int value) {
    stringstream ss;
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ss << SET_MV << " " << index << " " << mvIndex << " " << value;

    //Server always should have machineID=0
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    ss >> code;
    if(code != SUCCESS) {
        printf("MV is invalid");
    }
}

int GetSyscall(int index, int mvIndex) {
    stringstream ss;
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ss << GET_MV << " " << index << " " << mvIndex;

    //Server always should have machineID=0
    char * data = (char*)ss.str().c_str();
    pktHdr.to = rand() % NUM_SERVERS;
    pktHdr.from = myMachineID;
    mailHdr.to = 0;
    mailHdr.from = currentThread->myMailbox;
    mailHdr.length = strlen(data) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    char buffer[MaxMailSize];
    postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
    ss.clear();
    ss.str("");
    ss << buffer;
    int code;
    ss >> code;
    int value;
    if(code != SUCCESS) {
        printf("MV is invalid");
    } else {
        ss >> value;
        return value;
    }
    return 0;
}

void printIPT() {
    
    /*ipt[ppn].virtualPage = neededVPN;
     ipt[ppn].physicalPage = ppn;
     ipt[ppn].mySpace = currentThread->space;
     ipt[ppn].valid = TRUE;
     ipt[ppn].readOnly = FALSE;
     ipt[ppn].use = FALSE;
     ipt[ppn].dirty = FALSE;*/
    
    for(int i = 0; i < NumPhysPages;i++) {
        
        printf("IPT virtual page: %d IPT physical page: %d IPT space: %d IPT valid: %d IPT readOnly: %d IPT use: %d IPT dirty: %d\n", ipt[i].virtualPage, ipt[i].physicalPage, ipt[i].mySpace ,ipt[i].valid,ipt[i].readOnly,ipt[i].use,ipt[i].dirty);
    }
}

void printTLB() {
    
    /*machine->tlb[currentTLB].physicalPage = ipt[ppn].physicalPage;
     machine->tlb[currentTLB].virtualPage  = ipt[ppn].virtualPage;
     machine->tlb[currentTLB].readOnly  = ipt[ppn].readOnly;
     machine->tlb[currentTLB].dirty  = ipt[ppn].dirty;
     machine->tlb[currentTLB].valid = TRUE;
     machine->tlb[currentTLB].use  = ipt[ppn].use;*/
    
    for(int i = 0;i<4;i++) {
        
        printf("TLB physical page: %d TLB virtual page: %d TLB readonly: %d TLB dirty: %d TLB valid: %d TLB use: %d\n",machine->tlb[i].physicalPage, machine->tlb[i].virtualPage,machine->tlb[i].readOnly,machine->tlb[i].dirty,machine->tlb[i].valid,machine->tlb[i].use);
    }
}

int handleMemoryFull() {
    // choose page from IPT to evict
    
    int page = -1;
    
    if (pageReplacementPolicy == FIFO) {
        int * pagePointer = (int*)pageQueue->Remove();
        page = *pagePointer;
        delete pagePointer;
        
    }
    else {
        page = rand() % NumPhysPages;
    }
    
    // printf("Going to evict physical page: %d\n",page);
    
    for (int i = 0; i < 4; i++) {
        // propagate the dirty bit to the IPT and invalidate that TLB entry. Be sure to update the page table for the evicted page.
        if (machine->tlb[i].physicalPage == page && machine->tlb[i].valid) {
                ipt[page].dirty = machine->tlb[i].dirty;
                machine->tlb[i].valid = FALSE;
        }
    }

    // if dirty, WriteAt() to swap file
    if(ipt[page].dirty) {
        int location = swapFileBitMap->Find();
        // handle -1 from swapfile bitmap find (not likely but we have to do it anyways)
        if(location == -1) {
            
            printf("Swapfile bitmap could not find a page, make it bigger");
            interrupt->Halt();
        }
        
        location = location * PageSize;
        
        // printf("Writing to swapfile at location %d\n",location);
        
        swapFile->WriteAt(&(machine->mainMemory[page*PageSize]), PageSize, location);
        // tell the page table the bye offset of the evicted page
        ipt[page].mySpace->pageTable[ipt[page].virtualPage].byteOffset = location;
        ipt[page].mySpace->pageTable[ipt[page].virtualPage].diskLocation = SWAPFILE;
        ipt[page].mySpace->pageTable[ipt[page].virtualPage].dirty = TRUE;
        
    }
    
    ipt[page].mySpace->pageTable[ipt[page].virtualPage].valid = FALSE;
    
    // printf("In handle memory full:\n");
    // printIPT();
    // printTLB();
    // printf("\n\n");
    
    return page;
}

int handleIPTMiss (int neededVPN) {

    memoryLock->Acquire();
    int ppn = physicalPageBitMap->Find();
    memoryLock->Release();
    
    
    iptLock->Acquire();
    
    if(ppn == -1) {
        ppn = handleMemoryFull();
    }
    
    //update all ipt fields
    ipt[ppn].virtualPage = neededVPN;
    ipt[ppn].physicalPage = ppn;
    ipt[ppn].mySpace = currentThread->space;
    ipt[ppn].valid = TRUE;
    ipt[ppn].readOnly = FALSE;
    ipt[ppn].use = FALSE;
    ipt[ppn].dirty = FALSE;
    
    if(pageReplacementPolicy == FIFO) {
     
        int * ppnPointer = new int;
        
        *ppnPointer = ppn;
        
        pageQueue->Append((void*)ppnPointer);
    }
    
    if(currentThread->space->pageTable[neededVPN].diskLocation == EXECUTABLE && currentThread->space->pageTable[neededVPN].byteOffset != -1) {
        
        //only do this if necessary
        // printf("reading from executable at byte offset %d\n", currentThread->space->pageTable[neededVPN].byteOffset);
        
        currentThread->space->myExecutable->ReadAt(&(machine->mainMemory[ppn*PageSize]), PageSize, currentThread->space->pageTable[neededVPN].byteOffset);
    }
    else if(currentThread->space->pageTable[neededVPN].diskLocation == SWAPFILE && currentThread->space->pageTable[neededVPN].byteOffset != -1) { //Should a stack page be able to write to the swap file???
        
        
        // printf("reading from swapfile at byte offset %d\n", currentThread->space->pageTable[neededVPN].byteOffset);
        
        swapFile->ReadAt(&(machine->mainMemory[ppn*PageSize]), PageSize, currentThread->space->pageTable[neededVPN].byteOffset);
        
        swapFileBitMap->Clear(currentThread->space->pageTable[neededVPN].byteOffset/PageSize);
        ipt[ppn].dirty = TRUE;
    }
    
    //update PageTable
    currentThread->space->pageTable[neededVPN].physicalPage = ppn;
    currentThread->space->pageTable[neededVPN].valid = TRUE;
    currentThread->space->pageTable[neededVPN].virtualPage = neededVPN;
    currentThread->space->pageTable[neededVPN].use = FALSE;
    currentThread->space->pageTable[neededVPN].dirty = FALSE;
    currentThread->space->pageTable[neededVPN].readOnly = FALSE;
    
    // printf("In handle IPT miss:\n");
    // printIPT();
    // printTLB();
    // printf("\n\n");
    
    iptLock->Release();
    
    return ppn;
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
            case SC_CreateMV:
                DEBUG('a', "CreateMV syscall.\n");
                rv = CreateMVSyscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_DestroyMV:
                DEBUG('a', "DestroyMV syscall.\n");
                DestroyMVSyscall(machine->ReadRegister(4));
                break;
            case SC_Get:
                DEBUG('a', "Get syscall.\n");
                rv = GetSyscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
            case SC_Set:
                DEBUG('a', "Set syscall.\n");
                SetSyscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
        }
        
        // Put in the return value and increment the PC
        machine->WriteRegister(2,rv);
        machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
        
        return;
    }
    else if(which == PageFaultException) {
        
        
        IntStatus old = interrupt->SetLevel(IntOff);
        
        int VA = machine->ReadRegister(39);
        int VPN = VA/PageSize;
        
        // printf("Needed vpn: %d\n", VPN);
        
        int ppn = -1;
        for(int i = 0; i < NumPhysPages; i ++) {
            
            if(ipt[i].valid && ipt[i].virtualPage == VPN && ipt[i].mySpace == currentThread->space) {
                
                ppn = i;
                break;
            }
                
        }

        if(ppn == -1) {
            ppn = handleIPTMiss(VPN);
        }

        if(machine->tlb[currentTLB].valid) {
        	ipt[machine->tlb[currentTLB].physicalPage].dirty = machine->tlb[currentTLB].dirty;
        }   

        //now update TLB
        machine->tlb[currentTLB].physicalPage = ipt[ppn].physicalPage;
        machine->tlb[currentTLB].virtualPage  = ipt[ppn].virtualPage;
        machine->tlb[currentTLB].readOnly  = ipt[ppn].readOnly;
        machine->tlb[currentTLB].dirty  = ipt[ppn].dirty;
        machine->tlb[currentTLB].valid = TRUE;
        machine->tlb[currentTLB].use  = ipt[ppn].use;
        
        currentTLB = (currentTLB + 1) % TLBSize;
        interrupt->SetLevel(old);
    }
    else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
