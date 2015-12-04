#include "syscall.h"

int main()
{
    int i;
    for(i = 0; i < 5; i++) {
        Print("Executing createlock # %i", 25, i * 1000, 0);
        Exec("../test/createlock", 18);
    }
    Exit(0);
}