#include "syscall.h"

main()
{
    int i = Rand() % 100;
    Print("%i\n", 4, i * 1000, 0);
    Exit(0);
}
