
#include "syscall.h"
int a[3];
int b, c;

int main()
{
	Write("About to yield\n", 15, ConsoleOutput);
    Yield();
}
