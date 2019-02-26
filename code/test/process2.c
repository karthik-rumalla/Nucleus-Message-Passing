#include "syscall.h"

int
main()
{
	char *str = "Hi, Im from Process 2 sending back answer to Process1";
	int i;
	char *str1 = "Hi, Im from Process 2 sending message to Process3";
	int j;


	i = WaitMessage("process1",str,-1);
	SendAnswer(-1,str,i);

	j = SendMessage("process3",str1,-1);
	WaitAnswer(-1,str1,j);

	Exit(0);
}
