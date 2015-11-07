#include "syscall.h"

main()
{
    int l = 0;
    Release(l);
    Print("Released lock %i\n", 18, l * 1000, 0);
    Exit(0);
}