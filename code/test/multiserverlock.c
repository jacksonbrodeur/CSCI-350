#include "syscall.h"

main()
{
    int l = CreateLock("abc", 3);
    Print("Creating Lock at index: %i\n", 28, l * 1000, 0);
    Acquire(l);
    Print("Acquired lock %i\n", 18, l * 1000, 0);
    Release(l);
    Print("Released lock %i\n", 18, l * 1000, 0);

    Exit(0);
}