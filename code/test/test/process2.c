#include "syscall.h"

int
main()
{
	char *str = "Hello how are you 2?";
	int i = WaitMessage("process1",str,-1);
	char *str1 = "Im good 2";
	SendAnswer(-1 ,str1,i);
	Exit(0);
}
