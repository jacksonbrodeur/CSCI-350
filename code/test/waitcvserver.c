#include "syscall.h"

main()
{
    int l = 0;
    int cv = 0;
    Acquire(l);
    Print("Waiting on lock %i with condition %i\n",38, l * 1000 + cv, 0);
    Wait(cv, l);
    Release(l);
    Exit(0);
}