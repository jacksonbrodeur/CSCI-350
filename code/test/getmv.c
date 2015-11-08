#include "syscall.h"

int main()
{
    int mv = 0;
    int value = Get(mv);
    Print("Getting MV %i with value %i\n", 29, mv * 1000 + value, 0);
    Exit(0);
}