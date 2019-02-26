#include "syscall.h"

int
main()
{
	char *str = "Hi, Im from Process 1 sending message to Process2";
	int i;
	char *str1 = "Hi, Im from Process 1 sending message to Process3";
	int j;
	char *str2 = "Hi, Im from Process 1 sending message to Process3 again";
	int k;

	i = SendMessage("process2",str,-1);
	WaitAnswer(-1,str,i);

	j = SendMessage("process3",str1,-1);
	WaitAnswer(-1,str1,j);

	k = SendMessage("process3",str2,j);
	WaitAnswer(-1,str2,k);
	Exit(0);
}
