#include "syscall.h"

int
main()
{
	char *str = "Hi, Im from Process 7 sending back answer to Process4";
	int i;
	char *str1 = "Hi, Im from Process 7 sending message to Process5";
	int j;

	i = WaitMessage("process4",str,-1);
	SendAnswer(-1,str,i);

	j = SendMessage("process5",str1,-1);
	WaitAnswer(-1,str1,j);

	Exit(0);
}
