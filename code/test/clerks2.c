#include "syscall.h"

main()
{
    /*exec all the passport clerks and cashiers*/
    int i;
    for(i = 0; i < 5; i ++) {
        
        Exec("../test/passportclerk", 23);
        Exec("../test/cashier", 17);
    }
    
    Exit(0);
}
