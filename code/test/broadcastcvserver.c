#include "syscall.h"

main()
{
    int l = 0;
    int cv = 0;
    Acquire(l);
    Print("Broadcasting to lock %i with CV %i\n", 36, l * 1000 + cv, 0);
    Broadcast(cv, l);
    Release(l);
    Exit(0);
}