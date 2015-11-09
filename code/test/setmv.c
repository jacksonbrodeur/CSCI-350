#include "syscall.h"

int main()
{
    int mv = 0;
    int value = 10;
    Set(mv, value);
    Print("Setting MV %i to value %i\n", 27, mv * 1000 + value, 0);
    Exit(0);
}