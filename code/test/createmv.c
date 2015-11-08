#include "syscall.h"

int main()
{
    int mv = CreateMV("abc", 3);
    Print("Creating monitor at index: %i\n", 33, mv * 1000, 0);
    Exit(0);
}