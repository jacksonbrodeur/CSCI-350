#include "syscall.h"

main()
{
    /*exec 10 customers here*/
    int i;
    for(i = 0; i < 10; i ++) {
        
        Exec("../test/customer", 18);
    }
    
    Exit(0);
}
