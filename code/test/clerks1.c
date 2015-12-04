#include "syscall.h"

main()
{
    /*exec all the picture clerks and application clerks*/
    int i;
    for(i = 0; i < 5; i ++) {
        
        Exec("../test/pictureclerk",22);
        Exec("../test/applicationclerk", 26);
    }
    
    Exit(0);
}
