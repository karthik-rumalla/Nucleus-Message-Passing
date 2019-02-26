#include "syscall.h"

int
main()
{
	char *str = "Hi, Im from Process 3 sending back answer to Process1";
	int i;
	char *str1 = "Hi, Im from Process 3 sending back answer to Process2";
	int j;
	char *str2 = "Hi, Im from Process 3 sending back answer to Process1";
	int k;


	i = WaitMessage("process1",str,-1);
	SendAnswer(-1,str,i);

	j = WaitMessage("process2",str1,-1);
	SendAnswer(-1,str1,j);

	k = WaitMessage("process1",str2,-1);
	SendAnswer(-1,str2,k);

	Exit(0);
}
