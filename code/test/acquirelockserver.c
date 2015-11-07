#include "syscall.h"

main()
{
    int l = 0;
    Acquire(l);
    Print("Acquired lock %i\n", 18, l * 1000, 0);
    Exit(0);
}