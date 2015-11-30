#include "syscall.h"

main()
{
    int mv = CreateMV("mv", 2);
    int value;
    Print("Creating mv at index: %i\n", 26, mv * 1000, 0);
    Set(mv, 0, 100);
    Print("Set mv %i to %i\n", 17, mv * 1000 + 100, 0);
    value = Get(mv, 0);
    Print("Retrieved mv %i with value %i\n", 31, mv * 1000 + value, 0);

    Exit(0);
}