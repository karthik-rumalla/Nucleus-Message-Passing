#include "syscall.h"

int
main()
{
	char *str = "Hello how are you?";
	int i = SendMessage("process2",str,1);
	char *str1 = "Im good";
	WaitAnswer(-1,str1,i);
	Exit(0);
}
