#include "syscall.h"

int
main()
{
	char *str = "Hi, Im from Process 5 sending back answer to Process4";
	int i;
	char *str1 = "Hi, Im from Process 5 sending message to Process6";
	int j;
	char *str2 = "Hi, Im from Process 5 sending back answer to Process7";
	int k;

	i = WaitMessage("process4",str,-1);
	SendAnswer(-1,str,i);

	j = SendMessage("process6",str1,-1);
	WaitAnswer(-1,str1,j);

	k = WaitMessage("process7" , str2 , -1);

	Exit(0);
}
