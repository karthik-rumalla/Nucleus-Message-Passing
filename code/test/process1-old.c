#include "syscall.h"

int
main()
{
    char *str = "Hello how are you?";
    int i = SendMessage("process2",str,-1);
    char *str1; 
    char* s2;
    str1 = "Im good";
    s2 = "test message";
    WaitAnswer(-1,str1,i);
    i = SendMessage("process2", s2, i);
    WaitAnswer(-1,str1,i);
    // char *strrr = "Hello how are you? again!";
    Exit(0);
}
