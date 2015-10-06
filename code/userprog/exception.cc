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

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int validateLock(int index) {
    if (index < 0 || index > MAX_LOCKS) {
        printf("The index: %d was invalid.\n", index);
        return 0;
    }
    if (kernelLocks[index]->lock == NULL) {
        printf("There is no lock at index: %d\n", index);
        return 0;
    }
    if (kernelLocks[index]->addrSpace != currentThread->space) {
        printf("The lock trying to be accessed belongs to another thread.");
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
    if (kernelCVs[index]->addrSpace != currentThread->space) {
        printf("The condition trying to be accessed belongs to another thread.");
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

int ExecSyscall(int vaddr, int len) {
    
  return 0;
}

void ExitSyscall(int status) {

    //check if this thread is the last process
    
    currentThread->Finish();
}

void ForkSyscall(int vaddr) {

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

  int index = -1;
  for(int i = 0; i < MAX_LOCKS; i++) {
    if (kernelLocks[i]->lock == NULL) {
      kernelLocks[i] = kernelLock;
      index = i;
      break;
    }
  }

  DEBUG('d', "Creating Lock: %s\n", kernelLocks[index]->lock->getName());
  delete[] name;
  return index;
}

void DestroyLockSyscall(int index) {

    if(validateLock(index)) {
        if(!kernelLocks[index]->lock->isInUse()) {
            //see if the lock is busy, delete it here immediately if it is not
            kernelLocks[index]->lock = NULL;
        } else {
            //otherwise mark the lock for deletion and wait for people to stop using it
            kernelLocks[index]->isToBeDeleted = true;
        }
    }
    
}

int AcquireSyscall(int index) {

    if(validateLock(index)) {
        kernelLocks[index]->lock->Acquire();
        return 1;
    }
    return 0;
}

void ReleaseSyscall(int index) {

    if(validateLock(index)) {
        kernelLocks[index]->lock->Release();
    
        // if the lock is not in use (owner == NULL) then delete it (set lock to NULL)
        if(kernelLocks[index]->isToBeDeleted && !(kernelLocks[index]->lock->isInUse())) {
            kernelLocks[index]->lock = NULL;
        }
    }
}

int CreateConditionSyscall(int vaddr, int len) {
  
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
    
    DEBUG('d', "Creating Condition: %s\n", kernelCVs[index]->condition->getName());
    delete[] name;
    return index;
}

void DestroyConditionSyscall(int index) {
    if(validateCV(index)) {
        if(!kernelCVs[index]->condition->isInUse()) {
            //see if the CV is busy, delete it here immediately if it is not
            kernelCVs[index]->condition = NULL;
        } else {
            // otherwise mark the CV for deletion and wait for people to stop using it
            kernelCVs[index]->isToBeDeleted = true;
        }
    }    
}


void WaitSyscall(int conditionIndex, int lockIndex) {
    if(validateCV(conditionIndex) && validateLock(lockIndex)) {
        DEBUG('d', "Waiting on condition %s with lock %s\n", kernelCVs[conditionIndex]->condition->getName(),
            kernelLocks[lockIndex]->lock->getName());
        kernelCVs[conditionIndex]->condition->Wait(kernelLocks[lockIndex]->lock);
    }
}

void SignalSyscall(int conditionIndex, int lockIndex) {
    if(validateCV(conditionIndex) && validateLock(lockIndex)) {
        DEBUG('d', "Signalling on condition %s with lock %s\n", kernelCVs[conditionIndex]->condition->getName(),
            kernelLocks[lockIndex]->lock->getName());
        kernelCVs[conditionIndex]->condition->Signal(kernelLocks[lockIndex]->lock);
    }
}

void BroadcastSyscall(int conditionIndex, int lockIndex) {
    if(validateCV(conditionIndex) && validateLock(lockIndex)) {
        DEBUG('d', "Broadcasting on condition %s with lock %s\n", kernelCVs[conditionIndex]->condition->getName(),
            kernelLocks[lockIndex]->lock->getName());
        kernelCVs[conditionIndex]->condition->Broadcast(kernelLocks[lockIndex]->lock);
    }
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
