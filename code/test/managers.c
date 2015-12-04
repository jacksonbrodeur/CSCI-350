#include "syscall.h"

main()
{
    /*exec 1 manager here*/
    Exec("../test/manager", 17);
    
    Exit(0);
}
