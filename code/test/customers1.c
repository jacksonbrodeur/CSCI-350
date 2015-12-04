#include "syscall.h"

main()
{
    //exec 10 customers here
    for(int i = 0; i < 10; i ++) {
        
        Exec("../test/customer.c", 18);
    }
    
    Exit(0);
}
