#include "syscall.h"

main()
{
    int i = Rand() % 100;
    Print("Printing random number in range (0,99): %i\n", 44, i * 1000, 0);
    Exit(0);
}
