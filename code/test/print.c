#include "syscall.h"

int
main()
{
    int i = 5;
    int j = 10;
    Print("TestPrint: %i + %i = %i\n", 25, i * 1000 + j, (i+j) * 1000);
    Exit(0);
}