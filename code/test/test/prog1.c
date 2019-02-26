/*** prog1.c ***/
#include "syscall.h"

int
main()
{
	int child;
	child = Fork_POS(1);
	Wait_POS(child);
	child = Fork_POS(2);
	Wait_POS(child);
	child = Fork_POS(3);
	Wait_POS(child);
	Exit(0);
}

// /*** prog2.c ***/
// #include "syscall.h"

// int
// main()
// {
// 	int child1, child2;
// 	child1 = Fork_POS(1);
// 	child2 = Fork_POS(2);
// 	Wait_POS(child1);
// 	Wait_POS(child2);
// 	Exit(0);
// }

// /*** prog3.c ***/
// #include "syscall.h"

// int
// main()
// {
// 	Fork_POS(1);
// 	Fork_POS(2);
// 	Fork_POS(3);
// 	Exit(0);
// }
