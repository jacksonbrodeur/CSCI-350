#include "syscall.h"

int main(int argc, char const *argv[])
{
    int lock = CreateLock("lock", 4);
    int cv = CreateCondition("cv", 2);

    Acquire(lock);
    Print("Acquired lock %i\n", 18, lock * 1000, 0);
    Broadcast(cv, lock);
    Print("Broadcasted cv %i with lock %i\n", 32, cv * 1000 + lock, 0);
    Release(lock);
    Print("Released lock %i\n", 18, lock * 1000, 0);

    Exit(0);
    return 0;
}