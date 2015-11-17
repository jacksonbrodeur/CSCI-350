#include "syscall.h"

main()
{
    //exec all the passport clerks and cashiers
    for(int i = 0; i < 5; i ++) {
        
        Exec("../test/passportclerk.c", 23);
        Exec("../test/cashier.c", 17);
    }
    
    Exit(0);
}
