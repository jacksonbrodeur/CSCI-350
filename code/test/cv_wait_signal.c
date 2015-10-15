#include "syscall.h"

int cv;
int l;
int i;

void signalling_thread() {
    
    Acquire(l);
    Print("signalling_thread acquired lock %i\n", 36, l * 1000, 0);
    Signal(cv, l);
    Print("signalling_thread has signalled lock %i\n", 41, l*1000, 0);
    Release(l);
    
    Exit(0);
}

void waiting_thread() {
    
    Acquire(l);
    Print("waiting_thread is waiting on lock: %i\n", 39, l * 1000, 0);
    Wait(cv, l);
    Print("waiting_thread has been signalled\n", 35, 0, 0);
    Release(l);
    
    Exit(0);
}

main()
{
    l = CreateLock("lock",4);
    cv = CreateCondition("cv", 2);
    
    Fork(waiting_thread);
    Fork(signalling_thread);
    
    Exit(0);
}