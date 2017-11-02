#include <stdio.h>
#include <abc.h>

int func_add2(int a, int b)
{
#ifdef DEBUG
	printf("a = %d, b = %d\n", a, b);
#endif
	return (a+b);
}
