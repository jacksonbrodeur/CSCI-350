#include "syscall.h"

main()
{
    //exec all the picture clerks and application clerks
    for(int i = 0; i < 5; i ++) {
        
        Exec("../test/pictureclerk.c",22);
        Exec("../test/applicationclerk.c", 26);
    }
    
    Exit(0);
}
