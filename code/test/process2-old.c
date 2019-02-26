#include "syscall.h"

int
main()
{
	char *str;
	int result; 
	int i, j;
	char *str1;
	str = "Hello how are you 2?";
	str1 = "ACK";

	i = WaitMessage("process1",str,-1);
	SendAnswer(-1 ,str1,i); 
	j = WaitMessage("process1",str,-1);
	SendAnswer(-1 ,str1,j);
	Exit(0);
}