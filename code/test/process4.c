#include "syscall.h"

int
main()
{
	char *str = "Hi, Im from Process 4 sending message to Process5";
	int i;
	char *str1 = "Hi, Im from Process 4 sending message to Process6";
	int j;
	char *str2 = "Hi, Im from Process 4 sending message to Process7";
	int k;

	i = SendMessage("process5",str,-1);
	j = SendMessage("process6",str1,-1);
	k = SendMessage("process7",str2,-1);

	WaitAnswer(-1,str,j);
	WaitAnswer(-1,str1,k);
	WaitAnswer(-1,str2,i);

	Exit(0);
}
