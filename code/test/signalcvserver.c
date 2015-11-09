#include "syscall.h"

main()
{
    int l = 0;
    int cv = 0;
    Acquire(l);
    Signal(cv, l);
    Print("Signalled lock %i with condition %i\n", 37, l * 1000 + cv, 0);
    Release(l);
    Exit(0);
}